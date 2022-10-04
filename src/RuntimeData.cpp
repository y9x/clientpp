#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/allocators.h>
#include <rapidjson/schema.h>
#include <rapidjson/error/en.h>
#include <rapidjson/pointer.h>
#include "../utils/StringUtil.h"
#include "../utils/JsonUtil.h"
#include "./KrunkerWindow.h"
#include "./TraverseCopy.h"
#include "LoadRes.h"
#include "Log.h"
#include "resource.h"
#include <regex>

// adds an element to the string vector if not present
// returns false if the element is present, true if the element was pushed
template <class Element>
bool add_back(std::vector<Element> &vector, Element element)
{
  for (Element search : vector)
    if (search == element)
      return false;

  vector.push_back(element);
  return true;
}

std::regex meta_const(R"(const metadata\s*=\s*(\{[\s\S]+?\});)");

std::regex meta_comment(R"(\/{2}.*?\n|$|\/\*[\s\S]*?\*\/)");

std::regex us_export(R"(export function (\w+))");

// userscript struct?

void KrunkerWindow::load_userscripts()
{
  rapidjson::Document document;
  load_userscripts(document.GetAllocator());
}

rapidjson::Value KrunkerWindow::load_userscripts(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> allocator)
{
  additional_block_hosts.clear();
  additional_command_line.clear();

  rapidjson::Value result(rapidjson::kArrayType);

  rapidjson::Document default_userscript;

  {
    std::string sdefault_userscript;
    if (!load_resource(JSON_DEFAULT_USERSCRIPT, sdefault_userscript))
      clog::error << "Error loading default userscript" << clog::endl;

    rapidjson::ParseResult ok = default_userscript.Parse(sdefault_userscript.data(), sdefault_userscript.size());

    if (!ok)
      clog::error << "Error parsing default userscript: " << GetParseError_En(ok.Code()) << " (" << ok.Offset() << ")" << clog::endl;
  }

  rapidjson::SchemaDocument schema(default_userscript);

  for (IOUtil::WDirectoryIterator it(folder.directory + folder.p_scripts,
                                     L"*.js");
       ++it;)
  {
    rapidjson::Document metadata(rapidjson::kNullType);
    std::string buffer;

    if (IOUtil::read_file(it.path().c_str(), buffer))
    {

      std::vector<std::string> errors;

      std::smatch match;

      buffer =
          std::regex_replace(buffer, us_export, "exports.$1 = function $1");

      if (std::regex_search(buffer, match, meta_const))
      {
        std::string jsonMatch = std::regex_replace(match.str(1), meta_comment, "");

        rapidjson::ParseResult ok = metadata.Parse(jsonMatch.data(), jsonMatch.size());

        if (!ok)
          errors.push_back(std::string("Error parsing userscript: ") + GetParseError_En(ok.Code()) + " (" + std::to_string(ok.Offset()) + ")");
        else
        {
          buffer.replace(match[0].first, match[0].second,
                         "const metadata = _metadata;");

          rapidjson::SchemaValidator validator(schema);

          if (!metadata.Accept(validator))
          {
            // Input JSON is invalid according to the schema
            // Output diagnostic information
            rapidjson::StringBuffer sb;
            validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
            errors.push_back(std::string("Invalid schema: ") + sb.GetString());
            errors.push_back(std::string("Invalid keyword: ") + validator.GetInvalidSchemaKeyword());
            sb.Clear();
            validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
            errors.push_back(std::string("Invalid document: ") + sb.GetString());
          }

          // metadata = TraverseCopy(metadata, default_userscript, allocator);

          if (metadata.HasMember("features"))
          {
            if (metadata["features"].HasMember("block_hosts"))
              for (rapidjson::Value::ValueIterator it = metadata["features"]["block_hosts"].Begin(); it != metadata["features"]["block_hosts"].End(); ++it)
                add_back<std::wstring>(additional_block_hosts,
                                       JT::wstring(*it));

            if (metadata["features"].HasMember("command_line"))
              for (rapidjson::Value::ValueIterator it = metadata["features"]["command_line"].Begin(); it != metadata["features"]["command_line"].End(); ++it)
                add_back<std::wstring>(additional_command_line,
                                       JT::wstring(*it));

            if (metadata["features"].HasMember("config"))
            {
              bool changed = false;

              rapidjson::Document::AllocatorType folder_allocator = folder.config.GetAllocator();

              if (!folder.config["userscripts"].HasMember(rapidjson::Value(metadata["author"], folder_allocator)))
              {
                folder.config["userscripts"].AddMember(rapidjson::Value(metadata["author"], folder_allocator), rapidjson::Value(metadata["features"]["config"], folder_allocator), folder_allocator);
                changed = true;
              }
              else
              {
                rapidjson::Value config_value;
                folder.config["userscripts"][metadata["author"]] =
                    rapidjson::Value(TraverseCopy(folder.config["userscripts"][metadata["author"]], metadata["features"]["config"], folder_allocator, true, &changed), folder_allocator);
              }

              if (changed)
              {
                clog::info << "Changed" << clog::endl;
                folder.save_config();
              }
            }
          }
        }
      }

      rapidjson::Value row(rapidjson::kArrayType);
      std::string name = ST::string(it.file());
      row.PushBack(rapidjson::Value(name.data(), name.size(), allocator), allocator);
      row.PushBack(rapidjson::Value(buffer.data(), buffer.size(), allocator), allocator);
      row.PushBack(rapidjson::Value(metadata, allocator), allocator);
      rapidjson::Value errors_array(rapidjson::kArrayType);

      for (std::string error : errors)
      {
        clog::error << error << clog::endl;
        errors_array.PushBack(rapidjson::Value(error.data(), error.size(), allocator), allocator);
      }

      row.PushBack(errors_array, allocator);
      result.PushBack(row, allocator);
    }
    else
    {
      clog::error << "Failure reading userscript " << ST::string(it.path()) << clog::endl;
    }
  }

  return result;
}

rapidjson::Value KrunkerWindow::load_css(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> allocator)
{
  rapidjson::Value result(rapidjson::kArrayType);

  for (IOUtil::WDirectoryIterator it(folder.directory + folder.p_styles,
                                     L"*.css");
       ++it;)
  {
    std::string buffer;

    if (IOUtil::read_file(it.path().c_str(), buffer))
    {
      rapidjson::Value row(rapidjson::kArrayType);
      std::string name = ST::string(it.file());
      row.PushBack(rapidjson::Value(name.data(), name.size(), allocator), allocator);
      row.PushBack(rapidjson::Value(buffer.data(), buffer.size(), allocator), allocator);
      result.PushBack(row, allocator);
    }
  }

  std::string cli_css;
  if (load_resource(CSS_CLIENT, cli_css))
  {
    rapidjson::Value row(rapidjson::kArrayType);
    row.PushBack(rapidjson::Value("client/builtin.css", allocator), allocator);
    row.PushBack(rapidjson::Value(cli_css.data(), cli_css.size(), allocator), allocator);
    result.PushBack(row, allocator);
  }
  else
    clog::error << "Unable to load built-in CSS" << clog::endl;

  return result;
}

std::string KrunkerWindow::runtime_data()
{
  rapidjson::Document data(rapidjson::kObjectType);
  rapidjson::Document::AllocatorType allocator = data.GetAllocator();

  data.AddMember("config", rapidjson::Value(folder.config, allocator), allocator);
  data.AddMember("css", load_css(allocator), allocator);
  data.AddMember("js", load_userscripts(allocator), allocator);

  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  data.Accept(writer);

  return {buffer.GetString(), buffer.GetSize()};
}
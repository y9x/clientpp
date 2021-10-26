#include "./KrunkerWindow.h"
#include "./TraverseCopy.h"
#include "../Utils/StringUtil.h"
#include "Log.h"
#include "LoadRes.h"
#include <regex>
#include "resource.h"

using namespace StringUtil;
using JSON = nlohmann::json;

// adds an element to the string vector if not present
// returns false if the element is present, true if the element was pushed
template<class Element>
bool add_back(std::vector<Element>& vector, Element element) {
	for (Element search : vector)
		if (search == element) return false;

	vector.push_back(element);
	return true;
}

std::regex meta_const(R"(const metadata\s*=\s*(\{[\s\S]+?\});)");

std::regex meta_comment(R"(\/{2}.*?\n|$|\/\*[\s\S]*?\*\/)");

std::regex us_export(R"(export function (\w+))");

// userscript struct?

void KrunkerWindow::load_userscripts(JSON* data) {
	additional_block_hosts.clear();
	additional_command_line.clear();
	
	std::string sdefault_userscript;
	if (!load_resource(JSON_DEFAULT_USERSCRIPT, sdefault_userscript)) {
		clog::error << "Error loading default userscript" << clog::endl;
		return;
	}

	JSON default_userscript = JSON::parse(sdefault_userscript);

	for (IOUtil::WDirectoryIterator it(folder->directory + folder->p_scripts, L"*.js"); ++it;) {
		std::string buffer;

		if (IOUtil::read_file(it.path().c_str(), buffer)) {
			JSON put = JSON::array();
			std::smatch match;
			
			buffer = std::regex_replace(buffer, us_export, "exports.$1 = function $1");

			if (std::regex_search(buffer, match, meta_const)) {
				JSON metadata = default_userscript;
				std::vector<std::string> errors;
				
				try {
					std::string raw = std::regex_replace(match.str(1), meta_comment, "");
					
					// keep raw for loading ui controls and config
					JSON raw_metadata = JSON::parse(raw);
					metadata = TraverseCopy(raw_metadata, default_userscript);

					std::string author = metadata["author"];
					JSON& features = metadata["features"];
					JSON& userscripts = folder->config["userscripts"];
					JSON raw_features = JSON::object();

					if (raw_metadata.contains("features")) raw_features = raw_metadata["features"];
					if (raw_features.contains("gui")) features["gui"] = raw_features["gui"];
					if (raw_features.contains("config")) features["config"] = raw_features["config"];

					JSON config = features["config"];

					for (std::string host : features["block_hosts"])
						add_back<std::wstring>(additional_block_hosts, Convert::wstring(host));

					for (std::string cmd : features["command_line"])
						add_back<std::wstring>(additional_command_line, Convert::wstring(cmd));
					
					bool changed = false;

					if (!userscripts.contains(author)) {
						userscripts[author] = config;
						changed = true;
					}
					else userscripts[author] = TraverseCopy(userscripts[author], config, true, &changed);
					if (changed) {
						clog::info << "Changed" << clog::endl;
						folder->save_config();
					}

					buffer.replace(match[0].first, match[0].second, "const metadata = _metadata;");
				}
				catch (JSON::type_error err) {
					errors.push_back("Unable to read metadata from userscript " + Convert::string(it.file()) + ": " + err.what());
				}
				catch (JSON::parse_error err) {
					errors.push_back("Unable to process metadata from userscript " + Convert::string(it.file()) + ": " + err.what());
				}

				put[1] = metadata;

				if (errors.size()) {
					for (std::string error : errors)std::cout << error << std::endl;
					put[2] = errors;
				}
			}

			if (data) {
				put[0] = buffer;
				(*data)[Convert::string(it.file()).c_str()] = put;
			}
		}
	}
}

JSON KrunkerWindow::runtime_data() {
	JSON data = JSON::object();

	data["css"] = JSON::object();
	data["js"] = JSON::object();

	load_userscripts(&data["js"]);

	for (IOUtil::WDirectoryIterator it(folder->directory + folder->p_styles, L"*.js"); ++it;) {
		std::string buffer;
		if (IOUtil::read_file(it.path().c_str(), buffer))
			data["css"][Convert::string(it.file()).c_str()] = buffer;
	}

	data["css"]["client/hide.css"] = "img[src='./img/client.png'] { display: none !IMPORTANT; }";

	data["config"] = folder->config;

	return data;
}
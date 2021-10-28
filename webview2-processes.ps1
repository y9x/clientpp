foreach($inst in get-ciminstance Win32_Process -Filter "name = 'msedgewebview2.exe'"){
	write-output ""
	write-output $inst.commandline
	write-output ""
}
winreg = {
	source = path.join(dependencies.basePath, "winreg"),
}

function winreg.import()
	winreg.includes()
end

function winreg.includes()
	includedirs {
		path.join(winreg.source, "WinReg"),
	}
end

function winreg.project()

end

table.insert(dependencies, winreg)

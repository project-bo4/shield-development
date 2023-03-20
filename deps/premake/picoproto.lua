picoproto = {
	source = path.join(dependencies.basePath, "picoproto"),
}

function picoproto.import()
	links { "picoproto" }
	picoproto.includes()
end

function picoproto.includes()
	includedirs {
		picoproto.source
	}
end

function picoproto.project()
	project "picoproto"
		language "C"

		picoproto.includes()

		files {
			path.join(picoproto.source, "picoproto.cc"),
			path.join(picoproto.source, "picoproto.h"),
		}

		warnings "Off"
		kind "StaticLib"
end

table.insert(dependencies, picoproto)

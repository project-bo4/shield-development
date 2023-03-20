xxhash = {
	source = path.join(dependencies.basePath, "xxhash"),
}

function xxhash.import()
	xxhash.includes()
end

function xxhash.includes()
	includedirs {
		xxhash.source
	}
end

function xxhash.project()
	disablewarnings { "4244" }
end

table.insert(dependencies, xxhash)

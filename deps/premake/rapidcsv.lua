rapidcsv = {
	source = path.join(dependencies.basePath, "rapidcsv"),
}

function rapidcsv.import()
	rapidcsv.includes()
end

function rapidcsv.includes()
	includedirs {
		path.join(rapidcsv.source, "src"),
	}
end

function rapidcsv.project()
end

table.insert(dependencies, rapidcsv)

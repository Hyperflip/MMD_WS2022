--
-- Shared code for Playstation support for Visual Studio backend.
-- Copyright Blizzard Entertainment, Inc
--

local p = premake
local vstudio = p.vstudio
local vc2010 = p.vstudio.vc2010

p.modules.ps4 = {}

local m = p.modules.ps4

--
-- Methods
--

local function isPSConfig(cfg)
	return cfg.system and table.contains(os.getSystemTags(cfg.system), "sce")
end

local function isPsslFile(fname)
	return path.hasextension(fname, ".pssl")
end

local function fastMath(cfg)
	vc2010.element("FastMath", nil, tostring(p.config.isOptimizedBuild(cfg)))
end

function m.psslDefines(cfg, condition)
	if cfg.pssldefines and #cfg.pssldefines > 0 then
		local pssldefines = table.concat(cfg.pssldefines, ";")
		vc2010.element("PreprocessorDefinitions", condition, pssldefines .. ";%%(PreprocessorDefinitions)")
	end
end

function m.psslEntryPoint(cfg, condition)
	if cfg.psslentry then
		vc2010.element("EntryPoint", condition, cfg.psslentry)
	end
end

function m.psslOutput(cfg, condition)
	if cfg.pssloutput == "Embed" then
		vc2010.element("Embed", condition, "true")
	elseif cfg.pssloutput == "Header" then
		vc2010.element("GenerateHeader", condition, "true")
	end
end

function m.psslOutputHeader(cfg, condition)
	if cfg.pssloutputheader then
		vc2010.element("HeaderFileName", condition, cfg.pssloutputheader)
	end
end

function m.psslOutputFile(cfg, condition)
	if cfg.pssloutputfile then
		vc2010.element("OutputFileName", condition, cfg.pssloutputfile)
	end
end

function m.psslAdditionalOptions(cfg, condition)
	if #cfg.pssloptions > 0 then
		local opts = table.concat(cfg.pssloptions, " ")
		vc2010.element("AdditionalOptions", condition, '%s %%(AdditionalOptions)', opts)
	end
end

--
-- Properties
--

p.api.register {
	name = "pssldefines",
	scope = "config",
	kind = "list:string",
	tokens = true,
}

p.api.register {
	name = "pssloutput",
	scope = "config",
	kind = "string",
	allowed = {
		"Embed",
		"Header",
	}
}

p.api.register {
	name = "psslentry",
	scope = "config",
	kind = "string",
	tokens = true,
}

p.api.register {
	name = "pssloutputheader",
	scope = "config",
	kind = "string",
	tokens = true,
}

p.api.register {
	name = "pssloutputfile",
	scope = "config",
	kind = "string",
	tokens = true,
}

p.api.register {
	name = "pssloptions",
	scope = "config",
	kind = "list:string",
	tokens = true,
	pathVars = true,
}

--
-- Extensions
--

vc2010.elements.psslCompile = function(cfg)
	return {
		m.psslDefines,
		m.psslEntryPoint,
		m.psslOutput,
		m.psslOutputHeader,
		m.psslOutputFile,
		m.psslAdditionalOptions,
	}
end

function m.psslCompile(cfg)
	if isPSConfig(cfg) and p.config.hasFile(cfg, isPsslFile) then
		local contents = p.capture(function ()
			p.push()
			p.callArray(vc2010.elements.psslCompile, cfg)
			p.pop()
		end)

		if #contents > 0 then
			p.push('<WavePsslc>')
			p.outln(contents)
			p.pop('</WavePsslc>')
		end
	end
end

vc2010.categories.PSSL = {
	name = "PSSL",
	extensions = { ".pssl" },
	priority = 99,

	emitFiles = function(prj, group)
		local fileCfgFunc = function(fcfg, condition)
			if fcfg then
				return {
					vc2010.excludedFromBuild,
					m.psslDefines,
					m.psslEntryPoint,
					m.psslOutput,
					m.psslOutputHeader,
					m.psslOutputFile,
					m.psslAdditionalOptions,
				}
			else
				return {
					vc2010.excludedFromBuild,
				}
			end
		end

		vc2010.emitFiles(prj, group, "WavePsslc", {vc2010.fileType}, fileCfgFunc, function(cfg, fcfg)
			return isPSConfig(cfg) or isPSConfig(fcfg)
		end)
	end,

	emitFilter = function(prj, group)
		vc2010.filterGroup(prj, group, "WavePsslc")
	end,

	emitExtensionSettings = function(prj, group)
		p.w('<Import Project="$(VCTargetsPath)\\BuildCustomizations\\OrbisWavePsslc.props" />')
	end,

	emitExtensionTargets = function(prj, group)
		p.w('<Import Project="$(VCTargetsPath)\\BuildCustomizations\\OrbisWavePsslc.targets" />')
	end
}

--
-- Overrides
--

p.override(vc2010.elements, "itemDefinitionGroup", function(base, cfg)
	local items = base(cfg)
	if isPSConfig(cfg) then
		table.insertafter(items, vc2010.fxCompile, m.psslCompile)
	end

	return items
end)

p.override(vc2010, "platformToolset", function (base, cfg)
	if not isPSConfig(cfg) then
		return base(cfg)
	end
	vc2010.element("PlatformToolset", nil, "Clang")
end)

p.override(vc2010, "wholeProgramOptimization", function (base, cfg)
	if not isPSConfig(cfg) then
		return base(cfg)
	end

	if cfg.flags.LinkTimeOptimization then
		-- Note: On the PS, this is specified in the global flags
		vc2010.element("LinkTimeOptimization", nil, "true")
	end
end)

p.override(vc2010, "optimization", function (base, cfg, condition)
	if not isPSConfig(cfg) then
		return base(cfg, condition)
	end

	local map = { Off="Level0", On="Level1", Debug="Level0", Full="Level2", Size="Levels", Speed="Level3" }
	local value = map[cfg.optimize]
	if levelValue or not condition then
		vc2010.element('OptimizationLevel', condition, value or "Level0")
	end
	if cfg.flags.LinkTimeOptimization then
		-- PS link time optimization is specified in the CLCompile flags
		vc2010.element("LinkTimeOptimization", nil, "true")
	end
end)

p.override(vc2010, "treatWarningAsError", function (base, cfg)
	if not isPSConfig(cfg) then
		return base(cfg)
	end

	-- PS uses a different tag for treating warnings as errors
	if cfg.flags.FatalCompileWarnings and cfg.warnings ~= "Off" then
		vc2010.element("WarningsAsErrors", nil, "true")
	end
end)

p.override(vc2010, "warningLevel", function (base, cfg)
	if not isPSConfig(cfg) then
		return base(cfg)
	end

	-- PS uses a different tag for warning level
	local map = { Off = "WarningsOff", High = "MoreWarnings", Extra = "MoreWarnings"}
	vc2010.element("Warnings", nil, map[cfg.warnings] or "NormalWarnings")

	if cfg.warnings == "Extra" then
		vc2010.element("ExtraWarnings", nil, "true")
	end
end)

p.override(vc2010, "warningLevelFile", function (base, cfg, condition)
	if not isPSConfig(cfg) then
		return base(cfg, condition)
	end

	if cfg.warnings then
		-- PS uses a different tag for warning level
		local map = { Off = "WarningsOff", High = "MoreWarnings", Extra = "MoreWarnings"}
		vc2010.element("Warnings", condition, map[cfg.warnings] or "NormalWarnings")

		if cfg.warnings == "Extra" then
			vc2010.element("ExtraWarnings", nil, "true")
		end
	end
end)

p.override(vc2010, "debuggerFlavor", function (base, cfg)
	if not isPSConfig(cfg) then
		return base(cfg)
	end
	-- Does not set this at all.
end)


p.override(vc2010.elements, "clCompile", function(base, cfg)
	local calls = base(cfg)
	-- PS has GenerateDebugInformation and FastMath in the ClCompile section.
	if isPSConfig(cfg) then
		table.insert(calls, vc2010.generateDebugInformation)
		table.insert(calls, fastMath)
	end
	return calls
end)

p.override(vc2010, "generateDebugInformation", function(base, cfg)
	if not isPSConfig(cfg) then
		return base(cfg)
	end

	if cfg.symbols ~= nil then
		local map = {
			["On"]       = "true",
			["Off"]      = "false",
			["FastLink"] = "true",
			["Full"]     = "true",
		}
		if map[cfg.symbols] ~= nil then
			vc2010.element("GenerateDebugInformation", nil, map[cfg.symbols])
		end
	end
end)

p.override(vc2010, "languageStandard", function (base, cfg)
	if not isPSConfig(cfg) then
		return base(cfg)
	end

	if (cfg.cppdialect == "C++11" or cfg.cppdialect == "C++14") then
		vc2010.element("CppLanguageStd", nil, 'Cpp14')
	elseif (cfg.cppdialect == "C++17") then
		vc2010.element("CppLanguageStd", nil, 'Cpp17')
	end
end)

p.override(vc2010, "additionalCompileOptions", function(base, cfg, condition)
	if not isPSConfig(cfg) then
		return base(cfg, condition)
	end

	local opts = cfg.buildoptions
	if #opts > 0 then
		opts = table.concat(opts, " ")
		vc2010.element("AdditionalOptions", condition, '%s %%(AdditionalOptions)', opts)
	end
end)

p.override(vc2010, "clCompilePreprocessorDefinitions", function(base, cfg, condition)
	if not isPSConfig(cfg) then
		return base(cfg, condition)
	end

	vc2010.preprocessorDefinitions(cfg, cfg.defines, false, condition)
end)

p.override(vc2010, "exceptionHandling", function(base, cfg, condition)
	if not isPSConfig(cfg) then
		return base(cfg, condition)
	end

	if cfg.exceptionhandling == p.ON then
		vc2010.element("CppExceptions", condition, "true")
	elseif cfg.exceptionhandling == p.OFF then
		vc2010.element("CppExceptions", condition, "false")
	end
end)

p.override(vc2010, "runtimeTypeInfo", function(base, cfg, condition)
	if isPSConfig(cfg) and cfg.exceptionhandling == p.ON then
		vc2010.element("RuntimeTypeInfo", condition, "true")
	else
		return base(cfg, condition)
	end
end)

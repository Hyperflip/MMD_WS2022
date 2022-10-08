--
-- Playstation 4 support for Visual Studio backend.
-- Copyright Blizzard Entertainment, Inc
--

local p = premake
local vstudio = p.vstudio
local vc2010 = p.vstudio.vc2010
local m = p.modules.ps4

--
-- Extensions
--

p.ORBIS = "orbis"

if vstudio.vs2010_architectures ~= nil then
	vstudio.vs2010_architectures.orbis   = "ORBIS"
	p.api.addAllowed("system", p.ORBIS)

	os.systemTags[p.ORBIS] = { "orbis", "ps4", "sce", "console" }

	local osoption = p.option.get("os")
	if osoption ~= nil then
		table.insert(osoption.allowed, { p.ORBIS,  "Playstation 4" })
	end
end

filter { "system:Orbis" }
	toolset "clang"

filter { "system:Orbis", "kind:ConsoleApp or WindowedApp" }
	targetextension ".elf"

filter {}

--
-- Overrides
--

p.override(m, "psslOutput", function (base, cfg, condition)
	base(cfg, condition)

	-- Maintain old behavior for Orbis by setting custom defaults when not explicit
	-- This might be removed in future
	if cfg.system == p.ORBIS then
		if cfg.pssloutput == nil then
			vc2010.element("GenerateHeader", condition, "true")

			if cfg.pssloutputheader == nil then
				vc2010.element("HeaderFileName", condition, "$(ProjectDir)%%(Filename).h")
			end
		end
	end
end)

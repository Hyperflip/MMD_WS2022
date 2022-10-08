--
-- Console support for premake.
-- Copyright Blizzard Entertainment, Inc
--

if not premake.modules.ps4 then
	require('vstudio')
	premake.modules.ps4 = {}

	-- sce
	include 'playstation.lua'
	include 'orbis.lua'
end

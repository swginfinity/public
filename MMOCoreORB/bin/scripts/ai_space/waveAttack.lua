moveWaveattack = {
	{id="250448126",	name="AlwaysFailSpace",	pid="none"},
	{id="2047096269",	name="SelectorSpace",	pid="250448126"},
	{id="2726820896",	name="SequenceSpace",	pid="2047096269"},
	{id="3499818116",	name="IfSpace",	pid="2726820896"},
	{id="3004739680",	name="CheckShipDisabled",	pid="3499818116"},
	{id="2694634428",	name="SetDisabledEngineSpeed",	pid="2726820896"},
	{id="2357046776",	name="AlwaysFailSpace",	pid="2047096269"},
	{id="2103241960",	name="UpdateHomePosition",	pid="2357046776",	args={useTargetPosition=1}},
	{id="1232766482",	name="SequenceSpace",	pid="2047096269"},
	{id="2548093309",	name="IfSpace",	pid="1232766482"},
	{id="2990870941",	name="CheckDestination",	pid="2548093309"},
	{id="4112371132",	name="FindNextPosition",	pid="1232766482"}}
addAiTemplate("moveWaveattack", moveWaveattack)


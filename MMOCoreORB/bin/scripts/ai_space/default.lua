aggroDefault = {
	{id="1109954618",	name="SequenceSpace",	pid="none"},
	{id="2050102950",	name="IfSpace",	pid="1109954618"},
	{id="305546007",	name="CheckProspectAggression",	pid="2050102950"},
	{id="2593013780",	name="IfSpace",	pid="1109954618"},
	{id="2407318360",	name="CheckProspectLOS",	pid="2593013780"},
	{id="908237602",	name="SetDefenderFromProspect",	pid="1109954618"},
	{id="308570118",	name="SetMovementState",	pid="1109954618",	args={state=ATTACKING}}}
addAiTemplate("aggroDefault", aggroDefault)

attackDefault = {
	{id="1824859964",	name="SequenceSpace",	pid="none"},
	{id="159424751",	name="IfSpace",	pid="1824859964"},
	{id="597003156",	name="CheckRefireRate",	pid="159424751",	args={condition=500}},
	{id="1982099467",	name="IfSpace",	pid="1824859964"},
	{id="930829653",	name="CheckWeapons",	pid="1982099467"},
	{id="2510868304",	name="EngageSingleTarget",	pid="1824859964"}}
addAiTemplate("attackDefault", attackDefault)

awareDefault = {
	{id="1214404540",	name="SequenceSpace",	pid="none"},
	{id="1915611982",	name="NotSpace",	pid="1214404540"},
	{id="2125777402",	name="IfSpace",	pid="1915611982"},
	{id="3574561166",	name="CheckMovementState",	pid="2125777402",	args={condition=EVADING}},
	{id="2152617010",	name="LookForTargetSpace",	pid="1214404540"},
	{id="83649144",	name="SequenceSpace",	pid="2152617010"},
	{id="765304253",	name="CalculateAggroMod",	pid="83649144"},
	{id="4225492471",	name="IfSpace",	pid="83649144"},
	{id="2515118288",	name="CheckProspectInRange",	pid="4225492471"},
	{id="470242119",	name="AlwaysSucceedSpace",	pid="83649144"},
	{id="934410611",	name="TreeSocketSpace",	pid="470242119",	args={slot=LOOKATSPACE}},
	{id="769764285",	name="IfSpace",	pid="83649144"},
	{id="3977758222",	name="CheckMovementState",	pid="769764285",	args={condition=WATCHING}},
	{id="3090883828",	name="IfSpace",	pid="83649144"},
	{id="2582999451",	name="CheckAggroDelayPast",	pid="3090883828"},
	{id="235282565",	name="TreeSocketSpace",	pid="83649144",	args={slot=AGGROSPACE}}}
addAiTemplate("awareDefault", awareDefault)

evadeDefault = {
	{id="312238841",	name="SelectorSpace",	pid="none"},
	{id="2602567462",	name="SequenceSpace",	pid="312238841"},
	{id="4002815204",	name="IfSpace",	pid="2602567462"},
	{id="2649809160",	name="CheckMovementState",	pid="4002815204",	args={condition=EVADING}},
	{id="100606749",	name="AlwaysSucceedSpace",	pid="2602567462"},
	{id="1224718405",	name="SequenceSpace",	pid="100606749"},
	{id="2968774197",	name="IfSpace",	pid="1224718405"},
	{id="666306794",	name="CheckStopEvading",	pid="2968774197"},
	{id="1144510400",	name="SetMovementState",	pid="1224718405",	args={state=ATTACKING}},
	{id="688192376",	name="SequenceSpace",	pid="312238841"},
	{id="4108545327",	name="IfSpace",	pid="688192376"},
	{id="3895790068",	name="CheckMovementState",	pid="4108545327",	args={condition=ATTACKING}},
	{id="3972827323",	name="IfSpace",	pid="688192376"},
	{id="2565166350",	name="CheckEvadeChance",	pid="3972827323"},
	{id="3110218052",	name="SetMovementState",	pid="688192376",	args={state=EVADING}},
	{id="3267148840",	name="Evade",	pid="688192376",	args={evadeDelay=2000}}}
addAiTemplate("evadeDefault", evadeDefault)

idleDefault = {
	{id="3154429038",	name="SequenceSpace",	pid="none"},
	{id="1969087292",	name="NotSpace",	pid="3154429038"},
	{id="3447089863",	name="IfSpace",	pid="1969087292"},
	{id="1710296524",	name="CheckMovementState",	pid="3447089863",	args={condition=EVADING}},
	{id="2950938112",	name="NotSpace",	pid="3154429038"},
	{id="1976821924",	name="IfSpace",	pid="2950938112"},
	{id="2316913811",	name="CheckMovementState",	pid="1976821924",	args={condition=ATTACKING}},
	{id="3521391142",	name="NotSpace",	pid="3154429038"},
	{id="3390840483",	name="IfSpace",	pid="3521391142"},
	{id="2770349133",	name="CheckHasPatrol",	pid="3390840483"},
	{id="3954328391",	name="GeneratePatrol",	pid="3154429038",	args={distFromHome=1024.0, numPoints=5}},
	{id="2466828617",	name="NotSpace",	pid="3154429038"},
	{id="1745513383",	name="IfSpace",	pid="2466828617"},
	{id="3719992652",	name="CheckMovementState",	pid="1745513383",	args={condition=WATCHING}},
	{id="2700262997",	name="SetMovementState",	pid="3154429038",	args={state=PATROLLING}}}
addAiTemplate("idleDefault", idleDefault)

lookDefault = {
	{id="2349982047",	name="SequenceSpace",	pid="none"},
	{id="1015056356",	name="SelectorSpace",	pid="2349982047"},
	{id="2178900812",	name="IfSpace",	pid="1015056356"},
	{id="2737704374",	name="CheckMovementState",	pid="2178900812",	args={condition=OBLIVIOUS}},
	{id="3742455244",	name="IfSpace",	pid="1015056356"},
	{id="1948820794",	name="CheckMovementState",	pid="3742455244",	args={condition=PATROLLING}},
	{id="622023069",	name="SetMovementState",	pid="2349982047",	args={state=WATCHING}},
	{id="1344506751",	name="AlwaysSucceedSpace",	pid="2349982047"},
	{id="811185371",	name="SetAlert",	pid="1344506751",	args={aggroDelay=5.0}}}
addAiTemplate("lookDefault", lookDefault)

moveDefault = {
	{id="737776424",	name="SelectorSpace",	pid="none"},
	{id="2561598181",	name="SequenceSpace",	pid="737776424"},
	{id="1903740331",	name="IfSpace",	pid="2561598181"},
	{id="871654547",	name="CheckEngineSpeed",	pid="1903740331"},
	{id="904574168",	name="IfSpace",	pid="2561598181"},
	{id="323979800",	name="CheckRetreat",	pid="904574168",	args={condition=1500}},
	{id="2078642140",	name="SetMovementState",	pid="2561598181",	args={state=LEASHING}},
	{id="2987325598",	name="Leash",	pid="2561598181"},
	{id="3694068757",	name="AlwaysFailSpace",	pid="737776424"},
	{id="3770487453",	name="SelectorSpace",	pid="3694068757"},
	{id="1971265470",	name="SequenceSpace",	pid="3770487453"},
	{id="4059445686",	name="IfSpace",	pid="1971265470"},
	{id="1232444497",	name="CheckShipDisabled",	pid="4059445686"},
	{id="286243758",	name="SetDisabledEngineSpeed",	pid="1971265470"},
	{id="368441587",	name="SequenceSpace",	pid="3770487453"},
	{id="3848206840",	name="IfSpace",	pid="368441587"},
	{id="539785228",	name="CheckDestination",	pid="3848206840"},
	{id="1331314218",	name="FindNextPosition",	pid="368441587"}}
addAiTemplate("moveDefault", moveDefault)

rootDefault = {
	{id="2051740559",	name="SelectorSpace",	pid="none"},
	{id="1729558672",	name="TreeSocketSpace",	pid="2051740559",	args={slot=MOVESPACE}},
	{id="4225713865",	name="SequenceSpace",	pid="2051740559"},
	{id="2182549780",	name="SelectorSpace",	pid="4225713865"},
	{id="2962550237",	name="SequenceSpace",	pid="2182549780"},
	{id="464210289",	name="NotSpace",	pid="2962550237"},
	{id="2157444133",	name="IfSpace",	pid="464210289"},
	{id="1367447894",	name="CheckShipDisabled",	pid="2157444133"},
	{id="1447247403",	name="TreeSocketSpace",	pid="2962550237",	args={slot=EVADESPACE}},
	{id="763273353",	name="TreeSocketSpace",	pid="2182549780",	args={slot=TARGETSPACE}},
	{id="3449223772",	name="TreeSocketSpace",	pid="2182549780",	args={slot=AWARESPACE}},
	{id="3661746079",	name="IfSpace",	pid="2182549780"},
	{id="2975072207",	name="CheckMovementState",	pid="3661746079",	args={condition=ATTACKING}},
	{id="2621288157",	name="TreeSocketSpace",	pid="4225713865",	args={slot=ATTACKSPACE}},
	{id="869798738",	name="TreeSocketSpace",	pid="2051740559",	args={slot=IDLESPACE}}}
addAiTemplate("rootDefault", rootDefault)

targetDefault = {
	{id="1876748355",	name="SelectorSpace",	pid="none"},
	{id="2258474644",	name="SequenceSpace",	pid="1876748355"},
	{id="3051594756",	name="SelectorSpace",	pid="2258474644"},
	{id="4219427160",	name="GetProspectFromThreatMap",	pid="3051594756"},
	{id="1280314588",	name="SequenceSpace",	pid="3051594756"},
	{id="1485266457",	name="NotSpace",	pid="1280314588"},
	{id="3575964598",	name="IfSpace",	pid="1485266457"},
	{id="1056287644",	name="CheckMovementState",	pid="3575964598",	args={condition=FLEEING}},
	{id="260549053",	name="GetProspectFromDefenders",	pid="1280314588"},
	{id="3308935710",	name="SetDefenderFromProspect",	pid="3051594756"},
	{id="1453715206",	name="IfSpace",	pid="2258474644"},
	{id="1228992601",	name="CheckTargetIsValid",	pid="1453715206"},
	{id="3792515791",	name="SetMovementState",	pid="2258474644",	args={state=ATTACKING}},
	{id="1283033720",	name="AlwaysFailSpace",	pid="1876748355"},
	{id="627960272",	name="SequenceSpace",	pid="1283033720"},
	{id="3767183712",	name="EraseBlackboard",	pid="627960272",	args={param="aggroMod"}},
	{id="4195634024",	name="EraseBlackboard",	pid="627960272",	args={param="targetShipProspect"}},
	{id="3661684207",	name="ExitCombat",	pid="627960272"},
	{id="3553364391",	name="IfSpace",	pid="627960272"},
	{id="4107670032",	name="CheckMovementState",	pid="3553364391",	args={condition=ATTACKING}},
	{id="1537483837",	name="SetMovementState",	pid="627960272",	args={state=PATROLLING}}}
addAiTemplate("targetDefault", targetDefault)


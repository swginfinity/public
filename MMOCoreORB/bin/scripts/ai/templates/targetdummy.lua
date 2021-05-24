targetdummy = {
	{"root", "CompositeDefault", "none", SELECTORBEHAVIOR},
	{"idle", "CompositeDefault", "root", SEQUENCEBEHAVIOR},
	{"idle0", "MoveCreaturePet", "root", BEHAVIOR},
	{"idle1", "WaitDefault", "root", BEHAVIOR},
}

addAiTemplate("targetdummy", targetdummy)

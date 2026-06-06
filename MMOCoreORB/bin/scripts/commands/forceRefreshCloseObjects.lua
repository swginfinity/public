-- Infinity: GM diagnostic command for close objects debugging
-- Usage: /forceRefreshCloseObjects
-- Dumps close objects state, clears and rebuilds via updateZone(),
-- then reports stale/new objects. Full report sent via in-game mail.

ForceRefreshCloseObjectsCommand = {
    name = "forcerefreshcloseobjects",
    adminLevel = 15
}

AddCommand(ForceRefreshCloseObjectsCommand)

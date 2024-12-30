local projectName = "SteamHelper"

target(projectName)
    add_rules("ue4ss.mod")
    set_kind("binary")
    add_includedirs(".")
    add_files("**.cpp")


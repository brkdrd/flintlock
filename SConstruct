#!/usr/bin/env python
import os
import sys

EnsureSConsVersion(4, 0)

# Project settings
libname    = "godot_4d"
projectdir = "demo"
addon_dir  = "{}/addons/{}".format(projectdir, libname)

# Environment bootstrap
localEnv = Environment(tools=["default"], PLATFORM="")
customs  = ["custom.py"]
customs  = [os.path.abspath(p) for p in customs if os.path.exists(p)]

opts = Variables(customs, ARGUMENTS)
opts.Update(localEnv)
Help(opts.GenerateHelpText(localEnv))

env = localEnv.Clone()

if not (os.path.isdir("godot-cpp") and os.listdir("godot-cpp")):
    print("ERROR: godot-cpp submodule not initialised.")
    print("Run:  git submodule update --init --recursive")
    sys.exit(1)

# Build godot-cpp, then inherit its env
env = SConscript("godot-cpp/SConstruct", {"env": env, "customs": customs})

# Thirdparty includes
env.Append(CPPPATH=["thirdparty/eigen"])

# Extension sources
env.Append(CPPPATH=["src/"])

def collect_sources(dirs):
    src = []
    for d in dirs:
        src += Glob("{}/*.cpp".format(d))
    return src

sources = collect_sources([
    "src",
    "src/math",
    "src/nodes",
    "src/nodes/visual",
    "src/nodes/physics",
    "src/nodes/collision",
    "src/servers",
    "src/servers/physics",
    "src/servers/physics/core",
    "src/servers/visual",
    "src/resources",
    "src/editor",
])

# Optional: doc data (Godot 4.3+)
if env["target"] in ["editor", "template_debug"]:
    try:
        doc_data = env.GodotCPPDocData(
            "src/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml")
        )
        sources.append(doc_data)
    except AttributeError:
        pass

# Output paths
suffix = env["suffix"].replace(".dev", "").replace(".universal", "")
lib_filename = "{}{}{}{}".format(
    env.subst("$SHLIBPREFIX"), libname, suffix, env.subst("$SHLIBSUFFIX")
)
out_path = "{}/bin/{}".format(addon_dir, lib_filename)

library = env.SharedLibrary(out_path, source=sources)
Default(library)

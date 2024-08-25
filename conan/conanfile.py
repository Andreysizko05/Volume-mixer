
import os
from os.path import join

from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain
from conan.tools.env import VirtualRunEnv
from conan.tools.files import copy

class VolMixer(ConanFile):
    name = "volmixer"
    version = "0.1.0"
    settings = "os", "compiler", "build_type", "arch"
    build_policy = "missing"
    options = {
        "build_path": ["ANY"]
    }
    default_options = {
        "build_path": "abc"
    }

    def _layout(self):
        self.folders.root = ".."
        self.folders.source = "."
        if str(self.options.build_path) == "abc":
            self.folders.build = "build"
            self.folders.generators = "build"
        else:
            self.folders.build = str(self.options.build_path)
            self.folders.generators = str(self.options.build_path)

    if 1:#not os.environ.get('CONAN_FROM_CMAKE', False):
        layout = _layout

    def requirements(self):
        self.requires("qt/5.15.5")

    def configure(self):
        pass

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["CMAKE_FROM_CONAN"] = True
        deps = CMakeDeps(self)
        tc.generate()
        deps.generate()
        runenv = VirtualRunEnv(self)
        runenv.generate()
        for dep in self.dependencies.values():
            if len(dep.cpp_info.libdirs) > 0:
                copy(self, "*.dll", dep.cpp_info.libdirs[0], f'{self.folders.build}//{self.settings.build_type}')
                plugins_dir = join(dep.cpp_info.libdirs[0], "archdatadir", "plugins")
                copy(self, "*.dll", plugins_dir, f'{self.folders.build}//{self.settings.build_type}')
    
            if len(dep.cpp_info.bindirs) > 0:
                copy(self, "*.dll", dep.cpp_info.bindirs[0], f'{self.folders.build}//{self.settings.build_type}')
                plugins_dir = join(dep.cpp_info.bindirs[0], "archdatadir", "plugins")
                copy(self, "*.dll", plugins_dir, f'{self.folders.build}//{self.settings.build_type}')

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        target = None
        if not self.options.with_gui:
            target = "VolMixer"
        cmake.build(target=target)

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["VolMixer"]
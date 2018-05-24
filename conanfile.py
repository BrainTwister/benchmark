from conans import ConanFile, CMake

class RecordConan(ConanFile):
    
    name = "benchmark"
    version = "1.0"
    license = "MIT"
    description = "Accurate benchmark library for C++"
    homepage = "https://braintwister.eu"
    url = "https://github.com/braintwister/benchmark.git"
    
    exports_sources = "include/*", "test/*", "CMakeLists.txt"
    no_copy_source = True
    
    settings = "os", "compiler", "build_type", "arch"
    requires = \
        "gtest/1.8.0@bincrafters/stable", \
        "record/1.0@braintwister/testing"
    generators = "cmake"

    def build(self):
        # This is not building a library, just test
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.test()

    def package(self):
        self.copy("*.h")

    def package_id(self):
        self.info.header_only()

import os

from conans import ConanFile, CMake, tools


class JacyTestConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    requires = (('doctest/2.4.6', 'private'),)
    _source_subfolder = 'src'

    def build(self):
        cmake = CMake(self)
        # Current dir is "test_package/build/<build_id>" and CMakeLists.txt is
        # in "test_package"
        cmake.configure()
        cmake.build()

    def imports(self):
        pass

    def package(self):
        cmake_script_dirs = {
            'src': os.path.join(self._source_subfolder, 'scripts/cmake'),
            'dst': 'lib/cmake'
        }

        self.copy(pattern="doctest.cmake", **cmake_script_dirs)
        self.copy(pattern="doctestAddTests.cmake", **cmake_script_dirs)

    def test(self):
        if not tools.cross_building(self):
            os.chdir("bin")
            self.run(".%sexample" % os.sep)

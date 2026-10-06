"""Keep the window version and Windows EXE metadata in sync."""

import pathlib
import re
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]


class ReleaseVersionTests(unittest.TestCase):
    def test_cmake_and_windows_resource_versions_match(self):
        cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8-sig")
        version = re.search(r"VERSION\s+(\d+\.\d+\.\d+)", cmake).group(1)
        resource = (ROOT / "src/Resources/MHY_Scanner.rc").read_text(encoding="utf-8-sig")
        numeric = version.replace(".", ",") + ",0"
        for name in ("FILEVERSION", "PRODUCTVERSION"):
            self.assertRegex(resource, rf"{name}\s+{re.escape(numeric)}\b")
        for name in ("FileVersion", "ProductVersion"):
            self.assertIn(f'VALUE "{name}", "{version}"', resource)
        fixture = (ROOT / "tests/confirmation_delay/test_confirmation_ui.cpp").read_text(encoding="utf-8-sig")
        self.assertIn(f'ui.label_3->setText("{version}")', fixture)


if __name__ == "__main__":
    unittest.main()

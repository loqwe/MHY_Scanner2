"""Integration guards for the user-configurable login confirmation delay."""

import pathlib
import unittest
import xml.etree.ElementTree as ET

ROOT = pathlib.Path(__file__).resolve().parents[1]


class ConfirmationDelayTests(unittest.TestCase):
    def test_delay_is_editable_in_seconds(self):
        ui = ET.parse(ROOT / "src/UI/WindowMain.ui").getroot()
        widget = ui.find(".//widget[@name='spinConfirmDelay']")
        self.assertIsNotNone(widget)
        self.assertEqual(widget.get("class"), "QDoubleSpinBox")

    def test_both_scanners_route_auto_and_manual_through_ui(self):
        for name in ("QRCodeForScreen", "QRCodeForStream"):
            source = (ROOT / f"src/UI/{name}.cpp").read_text(encoding="utf-8-sig")
            self.assertNotIn('config["auto_login"]', source)
            self.assertNotIn('config.value("auto_login"', source)
            self.assertIn("Q_EMIT loginConfirm", source)

    def test_delay_is_saved_loaded_and_cancellable(self):
        source = (ROOT / "src/UI/WindowMain.cpp").read_text(encoding="utf-8-sig")
        self.assertIn('userinfo["confirm_delay_seconds"] = seconds', source)
        self.assertIn('userinfo.value("confirm_delay_seconds", 0.0)', source)
        self.assertIn("confirmationDelay.schedule", source)
        stop = source.split("void WindowMain::pBtStop()", 1)[1]
        self.assertIn("confirmationDelay.cancel()", stop)
        self.assertIn("confirmationJob.waitForFinished()", source)


if __name__ == "__main__":
    unittest.main()

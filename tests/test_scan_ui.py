"""Minimal scan-state UI guards without changing the existing interface."""

import pathlib
import unittest
import xml.etree.ElementTree as ET


ROOT = pathlib.Path(__file__).resolve().parents[1]
SOURCE = (ROOT / "src/UI/WindowMain.cpp").read_text(encoding="utf-8-sig")


class ScanUiTests(unittest.TestCase):
    def test_stream_running_feedback_waits_for_first_frame(self):
        self.assertIn("&QRCodeForStream::streamReady, this, &WindowMain::StartScanLive", SOURCE)
        start = SOURCE.split("void WindowMain::pBtStream", 1)[1].split(
            "void WindowMain::closeEvent", 1
        )[0]
        self.assertNotIn("emit StartScanLive()", start)
        self.assertLess(start.index("t2.stop()"), start.index("t2.wait()"))
        self.assertLess(start.index("t2.wait()"), start.index("t2.setUrl"))

    def test_capture_and_live_errors_use_their_own_mode(self):
        result = SOURCE.split("void WindowMain::islogin", 1)[1].split(
            "void WindowMain::loginConfirmTip", 1
        )[0]
        self.assertLess(result.index("const bool screenMonitoring"), result.index("pBtStop()"))
        self.assertIn('Show_QMessageBox("\u63d0\u793a", screenMonitoring ?', result)

    def test_original_ui_and_local_account_features_are_preserved(self):
        ui = ET.parse(ROOT / "src/UI/WindowMain.ui").getroot()
        live = ui.find(".//widget[@name='lineEditLiveId']")
        self.assertEqual(live.get("class"), "QComboBox")
        self.assertIsNone(ui.find(".//widget[@name='labelStatus']"))
        self.assertIn("setColumnCount(6)", SOURCE)
        self.assertIn("item->column() != 5", SOURCE)
        self.assertIn("startAccountSelfCheck();", SOURCE)
        self.assertIn("copyEntireRow(row);", SOURCE)
        self.assertIn("refreshLiveRoomIds();", SOURCE)
        self.assertIn("t1.setLoginInfo1(uid, stoken, mid)", SOURCE)
        self.assertIn("t2.setLoginInfo1(uid, stoken, mid)", SOURCE)
        self.assertIn("&QLineEdit::editingFinished", SOURCE)


if __name__ == "__main__":
    unittest.main()

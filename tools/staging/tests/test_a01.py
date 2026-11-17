"""Unit tests for A01's isolated-DB safety gates and deterministic report engine."""
from decimal import Decimal
import importlib.util
from pathlib import Path
import unittest

SCRIPT = Path(__file__).resolve().parents[1] / "a01.py"
spec = importlib.util.spec_from_file_location("a01", SCRIPT)
a01 = importlib.util.module_from_spec(spec)
spec.loader.exec_module(a01)


class A01Tests(unittest.TestCase):
    def test_project_name_cannot_target_production(self):
        for invalid in ("evemu", "server", "db", "evemu_a01", "evemu-a01-../../prod", "evemu-a01-ABC", "evemu-a01-"):
            with self.subTest(invalid=invalid), self.assertRaises(a01.StagingError):
                a01.validate_project(invalid)
        self.assertEqual(a01.validate_project("evemu-a01-ci-123"), "evemu-a01-ci-123")

    def test_compose_uses_unique_project_and_custom_file(self):
        cmd = a01.compose_cmd("evemu-a01-demo", "ps")
        self.assertEqual(cmd[:4], ["docker", "compose", "-p", "evemu-a01-demo"])
        self.assertTrue(str(a01.COMPOSE) in cmd)
        self.assertNotIn("--host", cmd)

    def test_game_sql_queries_are_allowlisted_by_numeric_ids(self):
        qs = a01.select_queries("", (90000001, 90000002), (98000001, 98000002))
        self.assertEqual(len(qs), 7)
        self.assertIn("FROM chrCharacters", qs["characters"])
        self.assertIn("FROM crpWalletDivisons", qs["corporation_wallets"])
        self.assertIn("FROM jnlCharacters", qs["wallet_journal"])
        self.assertIn("WHERE ownerID IN (90000001,90000002)", qs["items"])
        with self.assertRaises(a01.StagingError):
            a01.select_queries("prod.", (90000001, 90000002), (98000001, 98000002))
        with self.assertRaises(a01.StagingError):
            a01.select_queries("", (90000001, 90000001), (98000001, 98000002))

    def test_diff_wallet_and_rows(self):
        rows = {s: {} for s in a01.SCOPES}
        rows["characters"] = {"90010001": "100.00|98010001", "90010002": "200.00|98010002"}
        rows["wallet_journal"] = {"700": "90010001|10|0|0.00|100.00"}
        rows["items"] = {"111": "90010001|90010001|34|5|10"}
        before = {"format":"evemu-a01-v1", "mode":"fixture", "project":"evemu-a01-demo", "character_ids":[90010001,90010002], "corporation_ids":[98010001,98010002], "rows":rows}
        after = {**before, "rows": {**rows, "characters": {**rows["characters"], "90010001": "125.00|98010001"}, "items": {"111":"90010001|90010001|34|5|5", "222":"90010001|90010001|35|5|2"}, "wallet_journal": {"700": "90010001|10|0|0.00|100.00", "701":"90010001|10|100|25.00|125.00"}}}
        d = a01.diff(before, after)
        self.assertEqual(len(d["characters"]["changed"]), 1)
        self.assertEqual(len(d["items"]["added"]), 1)
        self.assertEqual(a01.wallet_delta(before, after, 90010001), Decimal("25.00"))
        self.assertEqual(a01.wallet_delta(before, after, 90010002), Decimal("0.00"))
        self.assertEqual(a01.journal_added(before, after, 90010001), 1)
        self.assertEqual(a01.journal_added(before, after, 90010002), 0)
        with self.assertRaises(a01.StagingError):
            a01.diff(before, {**after, "project":"evemu-a01-other"})

    def test_fixture_changes_cannot_touch_runtime_tables(self):
        for name in ("a01_fixture.sql", "a01_fixture_transition.sql"):
            sql = (a01.ROOT / "sql" / "staging" / name).read_text(encoding="utf-8")
            self.assertNotIn("UPDATE entity ", sql)
            self.assertNotIn("DELETE FROM entity ", sql)
            self.assertNotIn("DROP TABLE chrCharacters", sql)
            self.assertNotIn("UPDATE chrCharacters", sql)
            self.assertIn("a01_", sql)


if __name__ == "__main__":
    unittest.main()

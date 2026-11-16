"""Offline regression tests; no portrait downloads or database required."""
import gzip
import tempfile
import unittest
import zipfile
from pathlib import Path

from sync_portraits import (
    OfficialCDNOnly, agent_ids_from_dump, agent_ids_from_text,
    cached_portrait, is_jpeg, source_and_cache_roots, import_offline_portraits,
)


class AgentPortraitCacheTest(unittest.TestCase):
    def test_crucible_agent_ids_from_packaged_style_sql(self):
        with tempfile.TemporaryDirectory() as d:
            archive = Path(d) / "agtAgents.sql.gz"
            with gzip.open(archive, "wt", encoding="utf-8") as f:
                f.write("INSERT INTO agtAgents VALUES (3000000, 1, 2), (3010819, 4, 5);\n")
                f.write("INSERT INTO agtAgents VALUES (3999999, 7, 8);\n")
                f.write("-- nearby non-agent ID 4000000; solar system 30000053\n")
            self.assertEqual(agent_ids_from_dump(archive), [3000000, 3010819, 3999999])

    def test_invalid_and_duplicate_ids(self):
        with tempfile.TemporaryDirectory() as d:
            ids = Path(d) / "ids.txt"
            ids.write_text("3010819\n# comment\n3010819\n3000000\n")
            self.assertEqual(agent_ids_from_text(ids), [3000000, 3010819])
            ids.write_text("3010819\n90000000\n")
            with self.assertRaises(ValueError):
                agent_ids_from_text(ids)

    def test_jpeg_validation_and_missing_cache(self):
        with tempfile.TemporaryDirectory() as d:
            cache = Path(d)
            self.assertIsNone(cached_portrait(cache, 3010819))
            (cache / "3010819_256.jpg").write_bytes(b"not a jpeg")
            self.assertIsNone(cached_portrait(cache, 3010819))
            self.assertFalse(is_jpeg(b"\x89PNG" + b"x" * 150))
            self.assertTrue(is_jpeg(b"\xff\xd8" + b"x" * 200 + b"\xff\xd9"))
            (cache / "3010819_256.jpg").write_bytes(b"\xff\xd8" + b"x" * 200 + b"\xff\xd9")
            self.assertEqual(cached_portrait(cache, 3010819).name, "3010819_256.jpg")

    def test_import_owned_historical_jpeg_from_zip_offline(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            archive = root / "portraits.zip"
            jpeg = b"\xff\xd8" + b"original portrait bytes" * 10 + b"\xff\xd9"
            with zipfile.ZipFile(archive, "w") as z:
                z.writestr("portraits/3010819_256.jpg", jpeg)
                z.writestr("portraits/3999999_512.jpg", jpeg)
                z.writestr("../90000000_512.jpg", jpeg)
                z.writestr("../3010820_512.jpg", b"this is not JPEG")
            target = root / "cache"
            self.assertEqual(
                import_offline_portraits(archive, [3010819, 3999999, 3010820], target), 2)
            self.assertEqual((target / "3010819_256.jpg").read_bytes(), jpeg)
            self.assertTrue((target / "3999999_512.jpg").exists())
            self.assertFalse((target / "3010820_512.jpg").exists())
            self.assertEqual(
                import_offline_portraits(archive, [3010819, 3999999], target), 0)

    def test_bundled_fallback_is_a_real_jpeg(self):
        root = Path(__file__).resolve().parents[2]
        image = root / "assets/agent_portraits/agent_missing_512.jpg"
        self.assertTrue(image.is_file(), str(image))
        self.assertTrue(is_jpeg(image.read_bytes()))

    def test_source_root(self):
        source, cache = source_and_cache_roots(Path(__file__))
        self.assertEqual(source, cache)


if __name__ == "__main__":
    unittest.main()

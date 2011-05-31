using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using eveCache.Generation;
using eveMarshal;
using eveMarshal.Database;

namespace CacheToSQL
{
    class Program
    {
        private const string Indention = "    ";

        private static string GetSQLType(FieldType ft)
        {
            switch (ft)
            {
                case FieldType.I1:
                    return "TINYINT";
                case FieldType.I2:
                    return "SMALLINT";
                case FieldType.I4:
                    return "INT";
                case FieldType.CY:
                case FieldType.I8:
                case FieldType.FileTime:
                    return "BIGINT";
                case FieldType.UI1:
                    return "TINYINT UNSIGNED";
                case FieldType.UI2:
                    return "SMALLINT UNSIGNED";
                case FieldType.UI4:
                    return "INT UNSIGNED";
                case FieldType.UI8:
                    return "BIGINT UNSIGNED";
                case FieldType.R4:
                    return "FLOAT";
                case FieldType.R8:
                    return "DOUBLE";
                case FieldType.Bool:
                    return "BIT";
                case FieldType.Str:
                case FieldType.WStr:
                    return "TEXT";
                case FieldType.Bytes:
                    return "BLOB";
                default:
                    throw new ArgumentException("Can't translate to SQL type");
            }
        }

        private static bool IsColumnUnique(IEnumerable<PyPackedRow> rows, int index)
        {
            // worst case we actually need rows.Count entries
            var values = new List<long>(rows.Count());
            foreach (var row in rows)
            {
                long val = row.Columns[index].Value.IntValue;
                if (values.Contains(val))
                    return false;
                values.Add(val);
            }
            return true;
        }

        public class ModelData
        {
            public string Name { get; set; }
            public IEnumerable<PyPackedRow> Rows { get; set; }
            public PyPackedRow SingleRow { get; set; }
            public FieldType[] FieldTypes { get; set; }
            public string CreationQuery { get; set; }

            public static ModelData Create(CacheFile file)
            {
                var ret = new ModelData {Name = ShortenName(file.Name)};
                var rows = file.GetRows();
                ret.Rows = rows;
                if (rows.Count() == 0)
                    throw new InvalidDataException("Cache is empty");
                var singleRow = rows.First();
                ret.SingleRow = singleRow;

                // we need to go through all rows to find the maximum size type of all columns
                var fieldTypes = new FieldType[singleRow.Columns.Count];
                for (int i = 0; i < singleRow.Columns.Count; i++)
                    fieldTypes[i] = singleRow.Columns[i].Type;
                foreach (var row in rows)
                    for (int i = 0; i < row.Columns.Count; i++)
                        if (FieldTypeHelper.GetTypeBits(row.Columns[i].Type) > FieldTypeHelper.GetTypeBits(fieldTypes[i]))
                            fieldTypes[i] = row.Columns[i].Type;
                ret.FieldTypes = fieldTypes;

                // now build the creation query - fields, primary key, indices, cleanup
                var query = new StringBuilder();
                query.AppendLine("DROP TABLE IF EXISTS " + ret.Name + ";");
                query.AppendLine("CREATE TABLE " + ret.Name + " (");

                bool createAutoIncrement = !IsColumnUnique(rows, 0);
                string autoIncrementName = createAutoIncrement ? (singleRow.Columns.Any(c => c.Name == "id") ? "index" : "id") : "";
                if (createAutoIncrement)
                    query.AppendLine(Indention + autoIncrementName + " INT AUTO_INCREMENT,");

                for (int i = 0; i < singleRow.Columns.Count; i++)
                    query.AppendLine(Indention + singleRow.Columns[i].Name + " " + GetSQLType(fieldTypes[i]) + ",");

                if (createAutoIncrement)
                    query.AppendLine(Indention + "PRIMARY KEY (" + autoIncrementName + "),");
                else
                    query.AppendLine(Indention + "PRIMARY KEY (" + singleRow.Columns[0].Name + "),");

                for (int i = 1; i < singleRow.Columns.Count; i++)
                    if (singleRow.Columns[i].Name.EndsWith("ID"))
                        query.AppendLine(Indention + "INDEX " + singleRow.Columns[i].Name + " (" + singleRow.Columns[i].Name + "),");

                if (query[query.Length - 1] == '\n' && query[query.Length - 2] == '\r' && query[query.Length - 3] == ',')
                    query.Remove(query.Length - 3, 1);
                query.AppendLine(");");

                ret.CreationQuery = query.ToString();
                return ret;
            }
        }

        public static void CreateData(ModelData model, StreamWriter writer)
        {
            var insBuilder = new StringBuilder();
            insBuilder.Append("INSERT INTO " + model.Name + " (");
            foreach (var col in model.SingleRow.Columns)
                insBuilder.Append(col.Name + ", ");
            insBuilder.Length -= 2;
            insBuilder.Append(") VALUES (");
            var ins = insBuilder.ToString();

            foreach (var row in model.Rows)
            {
                writer.Write(ins);
                for (int i = 0; i < row.Columns.Count; i++)
                    writer.Write(GetColumnValue(row.Columns[i]) + (i < (row.Columns.Count - 1) ? ", " : ""));
                writer.WriteLine(");");
            }
        }

        private static string GetColumnValue(Column col)
        {
            if (col.Value == null)
                return "NULL";

            switch (col.Type)
            {
                case FieldType.Str:
                case FieldType.WStr:
                    if (!(col.Value is PyString))
                        return "NULL";
                    return "'" + col.Value.As<PyString>().Value + "'";

                case FieldType.R4:
                case FieldType.R8:
                    if (!(col.Value is PyFloat))
                        return "NULL";
                    return col.Value.As<PyFloat>().Value.ToString(CultureInfo.InvariantCulture);

                case FieldType.Bool:
                    return col.Value.IntValue == 0 ? "0" : "1";

                case FieldType.I8:
                    if (!(col.Value is PyLongLong))
                        return "NULL";
                    return col.Value.As<PyLongLong>().Value.ToString(CultureInfo.InvariantCulture);

                case FieldType.UI8:
                    if (!(col.Value is PyLongLong))
                        return "NULL";
                    return ((ulong)col.Value.As<PyLongLong>().Value).ToString(CultureInfo.InvariantCulture);

                case FieldType.I1:
                case FieldType.I2:
                case FieldType.I4:
                case FieldType.UI1:
                case FieldType.UI2:
                case FieldType.UI4:
                    return col.Value.IntValue.ToString(CultureInfo.InvariantCulture);
                    
                default:
                    throw new InvalidDataException("No proper representation for field type");
            }
        }

        private static string ShortenName(string longName)
        {
            if (longName.Contains(".") && !longName.EndsWith("."))
                return longName.Substring(longName.LastIndexOf('.') + 1);
            return longName;
        }

        static void Main()
        {
            CacheManager.LoadAll();

            var targets = new[] { "config.BulkData.bloodlineNames",
                "config.BulkData.locationscenes",
                "config.BulkData.overviewDefaults",
                "config.BulkData.schematicspinmap",
                "config.BulkData.overviewDefaultGroups",
                "config.BulkData.schematics",
                "config.BulkData.schematicstypemap",
                "config.BulkData.sounds",
                "config.BulkData.invtypematerials",
                "config.BulkData.ownericons"};

            foreach (var target in targets)
            {
                var cache = CacheManager.Get(target);
                if (cache == null)
                {
                    Console.WriteLine("Couldn't find cache file for target \"" + target + "\", skipping");
                    continue;
                }
                
                using (var sw = File.CreateText(cache.FileName + "-" + ShortenName(target) + ".sql"))
                {
                    sw.WriteLine("# Generated by CacheToSQL on " + DateTime.UtcNow + " (UTC)");
                    sw.WriteLine("# Source: " + cache.Name);
                    sw.WriteLine("# File: " + cache.FileName);
                    sw.WriteLine();

                    var modelData = ModelData.Create(cache);
                    sw.WriteLine(modelData.CreationQuery);
                    CreateData(modelData, sw);
                    sw.Flush();
                }
                Console.WriteLine("Processed " + target);
            }

            Console.WriteLine("All done");
            Console.ReadLine();
        }
    }
}

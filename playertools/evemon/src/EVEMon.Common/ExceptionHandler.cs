using System;
using System.Diagnostics;

namespace EVEMon.Common
{
    public static class ExceptionHandler
    {
        public static void LogException(Exception e, bool handled)
        {
            if (handled) LogException(e, "Handled exception");
            else LogException(e, "Unhandled exception");
        }

        public static void LogRethrowException(Exception e)
        {
            LogException(e, "Exception caught and rethrown");
        }

        private static void LogException(Exception e, string header)
        {
            Trace.WriteLine(String.Empty);
            EveClient.Trace(header);
            Trace.Indent();
            Trace.WriteLine(e.ToString());
            Trace.WriteLine(String.Empty);
            Trace.Unindent();
        }
    }
}
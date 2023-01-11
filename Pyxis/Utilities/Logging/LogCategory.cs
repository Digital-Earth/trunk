using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Utilities.Logging
{
    /// <summary>
    /// LogCategory is created for each Category we want to log: "PyxNet.Query" for example. or "WorldView.LibraryList" etc.
    /// LogCategory object can be Enabled or Disabled to control if we want to send those message to the LogRepository.
    /// </summary>
    public class LogCategory
    {
        public bool Enabled { get; set;}
        public string Category { get; private set; }

        private DateTime LastError = DateTime.MinValue;
        private int ErrorCount = 0;
        private int SkippedErrors = 0;

        public LogCategory(string category)
        {
            Category = category;
            Enabled = true;
        }

        public LogCategory(string category,bool enabled)
        {
            Category = category;
            Enabled = enabled;
        }

        public void Log(string key, string value)
        {
            if (Enabled)
            {
                LogRepository.Append(new LogRecord { Category = Category, Key = key, Value = value });
            }
        }

        public void Error(Exception ex)
        {
            if (Enabled)
            {
                Error(String.Format("[{0}] {1}", ex.GetType().Name, ex.Message));
            }
        }

        public void Error(string message)
        {
            if (Enabled)
            {
                if (ErrorCount > 0)
                {
                    //we have error in the last 30 sec...
                    if ((DateTime.Now - LastError) < TimeSpan.FromSeconds(30))
                    {
                        if (ErrorCount > 30)
                        {
                            //just keep collecting errors
                            LastError = DateTime.Now;
                            ErrorCount++;
                            SkippedErrors++;
                            return;
                        }
                    }
                    else //been more the 30 sec - reset error count
                    {
                        if (SkippedErrors > 0)
                        {
                            Log("Error", "two many errors (" + SkippedErrors + " errors weren't sent)");
                        }
                        ErrorCount = 0;
                        SkippedErrors = 0;
                    }
                }
                
                LastError = DateTime.Now;
                ErrorCount++;
                
                Log("Error", message);
            }
        }

        public void Warning(string message)
        {
            Log("Warning", message);
        }

        public void State(string value)
        {
            Log("State", value);
        }        
    }
}

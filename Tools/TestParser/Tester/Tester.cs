using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace TesterApplication
{
    public class Tester
    {
        public class OutputParser
        {
            private Stream m_parseStream;

            private TextWriter m_output;

            public OutputParser(Stream inputStream, TextWriter output)
            {
                m_parseStream = inputStream;
                m_output = output;
            }

            public void WriteCData(string message)
            {
                int index = message.IndexOf("![CDATA[", 0);
                while (index != -1)
                {
                    index += 8;
                    int endIndex = message.IndexOf("]]></", index);
                    if (endIndex != -1)
                    {
                        int length = endIndex - index;
                        if (length > 0)
                        {
                            m_output.WriteLine(message.Substring(index, length));
                        }
                        index = endIndex;
                    }

                    index = message.IndexOf("![CDATA[", index);
                }
            }

            public bool ParseTestOutput()
            {
                try
                {
                    byte[] testMem = new byte[m_parseStream.Length];
                    m_parseStream.Read(testMem, 0, (int)m_parseStream.Length);
                    string testLog = System.Text.UTF8Encoding.UTF8.GetString(testMem);

                    int index = testLog.IndexOf("success=\"False", 0);
                    while (index != -1)
                    {
                        index += 14;
                        m_output.Write("Failed test: ");
                        int failureIndex = testLog.IndexOf("<failure>", index);
                        int nameIndex = testLog.Substring(0, failureIndex).LastIndexOf("<test-case name=");
                        int endNameIndex = testLog.Substring(0, failureIndex).LastIndexOf("\" executed=\"True\"");
                        if (nameIndex != -1 && endNameIndex != -1)
                        {
                            nameIndex += 17;
                            int length = endNameIndex - nameIndex;
                            if (length > 0)
                            {
                                m_output.Write(testLog.Substring(nameIndex, length));
                            }
                        }
                        m_output.WriteLine();

                        if (failureIndex != -1)
                        {
                            int endOfFailure = testLog.IndexOf("</failure>", failureIndex);

                            int length = endOfFailure - failureIndex;
                            index = failureIndex;
                            if (length > 0)
                            {
                                WriteCData(testLog.Substring(failureIndex, length));
                                index += length;
                            }
                        }

                        // look for the next one
                        index = testLog.IndexOf("success=False", 0);
                    }

                }
                catch
                {
                    m_output.WriteLine("Parser blew up.");
                    return false;
                }
                return true;
            }

        }

        static void Main(string[] args)
        {
            try
            {
                FileInfo testLogInfo = new FileInfo(args[0]);
                FileStream testLogStream = testLogInfo.OpenRead();
                OutputParser parser = new OutputParser(testLogStream, System.Console.Out);
                parser.ParseTestOutput();
            }
            catch
            {
                System.Console.WriteLine("File not found {0}", args[0]);
            }
        }
    }
}

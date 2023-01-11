#region Copyright/Legal Notices
/*********************************************************
 * Copyright © 2008 HoytSoft. All rights reserved. 
 *	Please see included license for more details.
 *********************************************************/
#endregion

using System;
using System.Net.Sockets;
using System.Runtime.Serialization.Formatters.Binary;
using System.IO;

namespace HoytSoft.Common.UAC.Run {
	public class Program {
		static void printUsage() {
		}

		static void writeMsg(string msg) {
			//string dir = Environment.GetFolderPath(Environment.SpecialFolder.DesktopDirectory);
			//string file = Path.Combine(dir, "task.txt");
			//File.AppendAllText(file, msg + Environment.NewLine);
		}

		public static int Main(string[] args) {
			if (args == null || args.Length < 2) {
				printUsage();
				return -1;
			}

			string host = args[0];
			int port = -1;

			if (!int.TryParse(args[1], out port)) {
				printUsage();
				return -1;
			}

			BinaryFormatter formatter = new BinaryFormatter();
			NetworkStream stream = null;
			TcpClient c = null;

			try {
				c = new TcpClient();
				c.Connect(host, port);
				stream = c.GetStream();

				//Get the task from the UAC library
				//See HoytSoft.Common.UAC.UAC class, runIt() method
				object o = formatter.Deserialize(stream);
				if (!(o is IAdministrativeTask)) {
					printUsage();
					return -1;
				}

				//Here's the meat!
				IAdministrativeTask task = (o as IAdministrativeTask);
				TaskResult result = task.RunTask();

				formatter.Serialize(stream, result);
				stream.Flush();

			} catch {
				return -1;
			} finally {
				if (stream != null)
					stream.Close();
				if (c != null)
					c.Close();
			}

			return 0;
		}
	}
}

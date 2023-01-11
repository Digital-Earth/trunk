#region Copyright/Legal Notices
/***************************************************
 * Copyright © 2003 Roman Kuzmin. All rights reserved. 
 ***************************************************/
#endregion

using System;
using System.Collections.Generic;
using System.Text;
using System.CodeDom.Compiler;
using System.Diagnostics;
using Microsoft.CSharp;
using System.Reflection;

namespace HoytSoft.Common {
	public class Code {
		private static bool inArray(string value, string[] array) {
			if (value == null || array == null || array.Length <= 0)
				return false;
			foreach(string s in array) {
				if (value.Equals(s))
					return true;
			}
			return false;
		}

		private static bool inList(string value, IList<string> list) {
			if (value == null || list == null || list.Count <= 0)
				return false;
			foreach (string s in list) {
				if (value.Equals(s, StringComparison.InvariantCultureIgnoreCase))
					return true;
			}
			return false;
		}

		public static void EvalVoid(string expression) {
			EvalVoid("CSharp", expression, null);
		}

		public static void EvalVoid(string language, string expression) {
			EvalVoid(language, expression, null);
		}

		public static void EvalVoidWithLoadedAssemblies(string language, string expression, params string[] moreAssemblies) {
			Assembly[] currentAssemblies = System.AppDomain.CurrentDomain.GetAssemblies();
			List<string> assemblies = new List<string>(currentAssemblies.Length);

			//Cheats by loading the assembly into the app domain...
			foreach (string s in moreAssemblies) {
				Assembly assem = Assembly.LoadFile(s);
			}

			foreach(Assembly a in currentAssemblies) {
				assemblies.Add(a.ManifestModule.FullyQualifiedName);
			}

			EvalVoid(language, expression, assemblies.ToArray());
		}

		public static void EvalVoid(string language, string expression, params string[] assemblies) {
			if (string.IsNullOrEmpty(language)) {
				language = "CSharp";
				expression = string.Format("object Expression()\n{{ {0} return null;}}", expression);
			} else {
				switch (language.ToLower()) {
					case "c#":
						language = "CSharp";
						expression = string.Format("object Expression()\n{{ {0} return null;}}", expression);
						break;
					case "csharp":
						language = "CSharp";
						expression = string.Format("object Expression()\n{{ {0} return null;}}", expression);
						break;
					case "vb":
						language = "VisualBasic";
						expression = string.Format("Public Function Expression() As Object \n {0} \n Expression = null \n End Function", expression);
						break;
					case "visualbasic":
						language = "VisualBasic";
						expression = string.Format("Public Function Expression() As Object \n {0} \n Expression = null \n End Function", expression);
						break;
					case "vb.net":
						language = "VisualBasic";
						expression = string.Format("Public Function Expression() As Object \n {0} \n Expression = null \n End Function", expression);
						break;
					default:
						throw new ArgumentException("Unknown language: " + language);
				}
			}

			Execute(language, expression, false, null, null, assemblies);
		}

		public static object Eval(string expression) {
			return Eval(expression, false, null, null);
		}

		public static object Eval(string expression, params string[] assemblies) {
			return Eval(expression, false, null, null, assemblies);
		}

		// summary: Evaluates C# expression
		// expression: C# expression
		// mainAssembly: Add a reference to the main assembly
		// headCode: null or a code to be added at a generated source head
		// classBase: null or a generated class base list
		// assemblies: [params] null or additional assemblies
		// returns: Evaluated value as object
		public static object Eval(string expression, bool mainAssembly, string headCode, string classBase, params string[] assemblies) {
			string methodCode = String.Format("object Expression()\n{{return {0};}}", expression);
			return Execute("CSharp", methodCode, mainAssembly, headCode, classBase, assemblies);
		}

		// summary: Executes C# method's code
		// methodCode: Complete C# method's code
		// mainAssembly: Add a reference to the main assembly
		// headCode: null or a code to be added at a generated source head
		// classBase: null or a generated class base list
		// assemblies: [params] null or additional assemblies
		// returns: null or the executed method's return value
		public static object Execute(string language, string methodCode, bool mainAssembly, string headCode, string classBase, params string[] assemblies) {
			if (!"CSharp".Equals(language, StringComparison.InvariantCultureIgnoreCase))
				throw new ArgumentException("Language must be C#");

			// extract method name
			int i1, i2 = methodCode.IndexOf('(');
			if (i2 < 1)
				throw new Exception("Syntax error: ( expected");
			for (--i2; i2 >= 0 && methodCode[i2] <= ' '; --i2) ;
			for (i1 = i2; i1 >= 0 && methodCode[i1] > ' '; --i1) ;
			string methodName = methodCode.Substring(i1 + 1, i2 - i1);

			// code builder
			StringBuilder code = new StringBuilder();

			// << default using directives
			code.Append("using System;\n");

			// << head code
			if (headCode != null)
				code.AppendFormat("{0}\n", headCode);

			// << class name
			code.Append("public class AbsoluteClass");

			// << clase base list
			if (classBase != null)
				code.AppendFormat(" : {0}", classBase);

			// << class body with method
			code.AppendFormat("\n{{\npublic {0}\n}}\n", methodCode);

			// compiler parameters
			CompilerParameters parameters = new CompilerParameters();
			parameters.GenerateExecutable = false;
			parameters.GenerateInMemory = true;

			// referenced asseblies
			//parameters.ReferencedAssemblies.Add("System.dll");

			if (mainAssembly)
				parameters.ReferencedAssemblies.Add(
					Process.GetCurrentProcess().MainModule.FileName);
			if (assemblies != null)
				parameters.ReferencedAssemblies.AddRange(assemblies);
			
			// compiler and compilation
			ICodeCompiler compiler = CodeDomProvider.CreateProvider(language).CreateCompiler();
			CompilerResults results =
				compiler.CompileAssemblyFromSource(
				parameters, code.ToString());

			// errors?
			if (results.Errors.HasErrors) {
				StringBuilder error = new StringBuilder();
				error.Append("Compilation errors:\n");
				foreach (CompilerError err in results.Errors)
					error.AppendFormat(
						"#{0} {1}. Line {2}.\n",
						err.ErrorNumber, err.ErrorText, err.Line);
				error.AppendFormat("\n{0}", code);
				throw new Exception(error.ToString());
			}

			// class instance
			object classInstance =
				results.CompiledAssembly.CreateInstance("AbsoluteClass");

			// execute method
			MethodInfo info = classInstance.GetType().GetMethod(methodName);
			return info.Invoke(classInstance, null);
		}
	}
}

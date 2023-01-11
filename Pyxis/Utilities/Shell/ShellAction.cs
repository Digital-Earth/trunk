using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Utilities.Shell
{
    public class ShellAction
    {
        public String Description { get; set; }
        public List<string> Arguments { get; set; }
        public Action<string[]> Action { get; set; }

        public ShellAction(string description, List<string> arguments, Action<string[]> action)
        {
            Description = description;
            Arguments = arguments;
            Action = action;
        }

        public ShellAction(string description, object instance, MethodInfo methodInfo)
        {
            Description = description;
            Arguments = methodInfo.GetParameters().Select(x => x.Name).ToList();
            if (methodInfo.IsStatic)
            {
                Action = actionArgs => methodInfo.Invoke(null, ConvertMethodArgs(methodInfo, actionArgs));
            }
            else
            {
                if (instance == null)
                {
                    throw new ArgumentNullException("instance", "can't create shell action for non-static method");
                }
                Action = actionArgs => methodInfo.Invoke(instance, ConvertMethodArgs(methodInfo, actionArgs));
            }
        }

        public ShellAction(string description, MethodInfo methodInfo) : this(description,null,methodInfo)
        {
        }
        
        private object[] ConvertMethodArgs(System.Reflection.MethodInfo method, string[] args)
        {
            return method.GetParameters().Zip(args,(param,arg)=>TypeDescriptor.GetConverter(param.ParameterType).ConvertFromInvariantString(arg)).ToArray();            

        }
    }

    public class ShellActionAttribute : Attribute
    {
        public string Name { get; set; }
        public string Description { get; set; }
    }
}

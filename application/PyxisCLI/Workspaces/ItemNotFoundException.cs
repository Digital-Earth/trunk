using System;

namespace PyxisCLI.Workspaces
{
    public class ItemNotFoundException : Exception
    {
        public ItemNotFoundException(string message) : base(message)
        {
            
        }

        public ItemNotFoundException(string message, Exception inner) : base(message,inner)
        {            
        }        
    }
}
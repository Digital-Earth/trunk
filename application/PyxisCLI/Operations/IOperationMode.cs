namespace PyxisCLI.Operations
{
    interface IOperationMode
    {
        string Command { get; }

        string Description { get;  }
        void Run(string[] args);
    }
}

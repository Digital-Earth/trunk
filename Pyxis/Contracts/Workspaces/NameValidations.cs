using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Contract.Workspaces
{
    public interface INameValidator
    {
        bool ValidateWorkspaceName(string name);
        bool ValidateWorkspaceItemName(string name);
        bool ValidateFieldName(string name);
    }

    public class NameValidations : INameValidator
    {
        private bool ValidChar(char c)
        {
            return char.IsLetterOrDigit(c) || c == '_';
        }

        public bool ValidateWorkspaceName(string name)
        {
            return name.All(ValidChar);
        }

        public bool ValidateWorkspaceItemName(string name)
        {
            return name.All(ValidChar);
        }

        public bool ValidateFieldName(string name)
        {
            return name.All(c => ValidChar(c) || c == ' ');
        }
    }
}

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace ApplicationUtility
{
    public partial class UsernameAndPasswordForm : Form
    {
        public UsernameAndPasswordForm()
        {
            InitializeComponent();
        }

        private void UsernameAndPasswordForm_Load(object sender, EventArgs e)
        {
            passwordTextBox.PasswordChar = Char.ConvertFromUtf32(0x25CF)[0];
        }

        public string Username
        {
            get { return usernameTextBox.Text; }
            set { usernameTextBox.Text = value; }
        }

        public string Password
        {
            get { return passwordTextBox.Text; }
            set { usernameTextBox.Text = value; }
        }

        public string Message
        {
            get { return messageLabel.Text; }
            set { messageLabel.Text = value; } 
        }

        public class UsernameAndPassword
        {
            public string Username { get; set; }
            public string Password { get; set; }
            public string Target { get; set; }

            /// <summary>
            /// Ask the user to enter username and password information
            /// </summary>
            /// <returns>true if the user entered username and password</returns>
            public bool AskUser()
            {
                var result = false;

                Visualization.UIThread.Invoke(
                    (MethodInvoker)(()=>
                                        {
                                            var form = new UsernameAndPasswordForm();

                                            form.Message = "username and password required for\n " + Target;
                                            if (!String.IsNullOrEmpty(Username))
                                            {
                                                form.Username = Username;
                                            }
                                            if (!String.IsNullOrEmpty(Password))
                                            {
                                                form.Username = Password;
                                            }
                                            if (form.ShowDialog(Visualization.UIThread.Parent) == DialogResult.OK)
                                            {
                                                Username = form.Username;
                                                Password = form.Password;
                                                result = true;
                                            }
                                        }));
                return result;
            }
        }

        private void okButton_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.OK;
        }
    }
}

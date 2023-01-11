using System;
using System.Collections;
using System.Reflection;
using System.Windows.Forms;
using Pyxis.Utilities;

namespace PyxNet.Diagnostics
{
    public partial class TrafficListenerForm : Form
    {
        public TrafficListener Listener { get; set; }
        public TrafficListenerForm(TrafficListener listener)
        {
            Listener = listener;
            InitializeComponent();
        }

        private void StartButton_Click(object sender, EventArgs e)
        {
            Listener.StartRecording();
            StartButton.Enabled = false;
            StopButton.Enabled = true;
        }

        private void StopButton_Click(object sender, EventArgs e)
        {
            Listener.StopRecording();
            StartButton.Enabled = true;
            StopButton.Enabled = false;
        }

        private void TrafficListenerForm_Load(object sender, EventArgs e)
        {
            if (Listener.Recording)
            {
                StartButton.Enabled = false;
            }
            else
            {
                StopButton.Enabled = false;
            }

            Listener.Messages.CountChanged += MessagesOnCountChanged;
        }

        private void MessagesOnCountChanged(object sender, DynamicList<TrafficListener.RecordedMessage>.CountChangedEventArgs countChangedEventArgs)
        {
            UpdateList();
        }

        private delegate void UpdateListDelegate();
        private void UpdateList()
        {
            if (InvokeRequired)
            {
                BeginInvoke(new UpdateListDelegate(UpdateList));
                return;
            }

            MessageListView.BeginUpdate();

            try
            {


                int index = 0;
                foreach (var message in Listener.Messages)
                {
                    if (index >= MessageListView.Items.Count)
                    {
                        MessageListView.Items.Add(CreateListViewItem(message));
                    }
                    else
                    {
                        UpdateListViewItemFrom(MessageListView.Items[index], message);
                    }
                    index++;
                }

                while (index < MessageListView.Items.Count)
                {
                    Console.Write("index value :" + index + " " + MessageListView.Items.Count);
                    MessageListView.Items.RemoveAt(index);
                }
            }
            catch (Exception)
            {
                throw;
            }
            finally
            {
                MessageListView.EndUpdate();
            }
        }

        private void UpdateListViewItemFrom(ListViewItem listViewItem, TrafficListener.RecordedMessage recordedMessage)
        {
            if (listViewItem.Tag != recordedMessage)
            {
                listViewItem.SubItems[0].Text = recordedMessage.TimeStamp.ToLongTimeString();
                listViewItem.SubItems[1].Text = (recordedMessage.Source != null) ? recordedMessage.Source.FriendlyName : "None";
                listViewItem.SubItems[2].Text = (recordedMessage.Destination!=null)?recordedMessage.Destination.FriendlyName:"All";
                listViewItem.SubItems[3].Text = recordedMessage.Message.Identifier;
                listViewItem.SubItems[4].Text = recordedMessage.Message.Length.ToString();

                listViewItem.Tag = recordedMessage;
            }

            listViewItem.SubItems[5].Text = recordedMessage.Description;
        }

        private ListViewItem CreateListViewItem(TrafficListener.RecordedMessage recordedMessage)
        {
            var result = new ListViewItem(new string[]{
                recordedMessage.TimeStamp.ToLongTimeString(),
                (recordedMessage.Source!=null)?recordedMessage.Source.FriendlyName:"N.A",
                (recordedMessage.Destination!=null)?recordedMessage.Destination.FriendlyName:"N.A",
                recordedMessage.Message.Identifier,
                recordedMessage.Message.Length.ToString(),
                recordedMessage.Description
            });

            result.Tag = recordedMessage;

            return result;
        }

        private void MessageListView_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (MessageListView.SelectedItems.Count > 0)
            {
                ViewMessage(MessageListView.SelectedItems[0]);
            }
        }

        private void ViewMessage(ListViewItem selectedItem)
        {
            var recordedMessage = selectedItem.Tag as TrafficListener.RecordedMessage;

            BuildMessageTree(recordedMessage);
            ShowMessageBytes(recordedMessage.Message);

            UpdateListViewItemFrom(selectedItem,recordedMessage);
        }

        private void ShowMessageBytes(Message message)
        {
            var text = "";
            var bytes = message.Bytes.Array;

            for (int i = 0; i < Math.Min(5000,message.Length);i+=8)
            {
                var hex = "";
                var ascii = "";
                for(int c=0;c<=8;c++)
                {
                    if (i+c < bytes.Length)
                    {
                        var b = bytes[i + c];

                        hex += String.Format("{0:x2} ", b);

                        if (b >= 32 && b < 127)
                        {
                            ascii += char.ConvertFromUtf32(b);
                        }
                        else
                        {
                            ascii += ".";
                            //ascii += char.ConvertFromUtf32(9744); //BALLOT BOX
                        }
                    }
                }

                text += hex + "  " + ascii +"\r\n";
            }
            MessageBitsTextBox.Text = text;
        }

        private void AddMessageToTree(TrafficListener.RecordedMessage recordedMessage, Message message)
        {
            var decodedMessage = GenericMessageDecorder.TryDecodeMessage(message);

            if (decodedMessage != null)
            {
                var description = GenericMessageDecorder.TryBuildDescription(decodedMessage);
                if (!String.IsNullOrEmpty(description))
                {
                    recordedMessage.Description += description + " ";
                }

                var messageType = decodedMessage.GetType();

                var objectNode = MessageTreeView.Nodes.Add(messageType.Name);

                foreach (var prop in messageType.GetProperties(BindingFlags.Public | BindingFlags.Instance))
                {
                    try
                    {
                        if (typeof(Message).IsAssignableFrom(prop.PropertyType))
                        {
                            AddMessageToTree(recordedMessage, prop.GetValue(decodedMessage,null) as Message);
                        }
                        else
                        {
                            var propValue = prop.GetValue(decodedMessage, null);

                            if (propValue == null)
                            {
                                objectNode.Nodes.Add(prop.Name + ": null");
                            }
                            else
                            {
                                if (!(propValue is string) && propValue is IEnumerable)
                                {
                                    var list = propValue as IEnumerable;

                                    var listNode = objectNode.Nodes.Add(prop.Name);

                                    foreach(var item in list)
                                    {
                                        if (item == null)
                                        {
                                            listNode.Nodes.Add("null");
                                        }
                                        else
                                        {
                                            listNode.Nodes.Add(item.ToString());
                                        }
                                    }
                                }
                                else
                                {
                                    objectNode.Nodes.Add(prop.Name + ": " + propValue.ToString());
                                }
                            }
                        }
                    }
                    catch
                    {
                    }
                }
            }
        }

        private void BuildMessageTree(TrafficListener.RecordedMessage recordedMessage)
        {
            MessageTreeView.Nodes.Clear();

            recordedMessage.Description = "";
            AddMessageToTree(recordedMessage,recordedMessage.Message);

            MessageTreeView.ExpandAll();
        }
    }
}

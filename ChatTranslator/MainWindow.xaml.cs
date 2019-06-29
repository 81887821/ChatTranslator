using ChatTranslator.FFXIV;
using ChatTranslator.Translator;
using System;
using System.Diagnostics;
using System.Windows;
using System.Windows.Input;

namespace ChatTranslator
{
    public partial class MainWindow : Window
    {
        enum MainWindowState { ProcessDisconnected, MacroNotSelected, AllAvailable }

        private Key macroKey = Key.None;
        private bool macroShortcutEditing = false;
        private ITranslator translatorJ2K;
        private ITranslator translatorK2J;
        private FFXIVConnector connector = null;

        public MainWindow()
        {
            InitializeComponent();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            SetWindowState(MainWindowState.ProcessDisconnected);

            translatorJ2K = new DummyTranslator();
            translatorK2J = new DummyTranslator();

            UpdateProcessList();
        }

        private void TranslateButton_Click(object sender, RoutedEventArgs e)
        {
            chatInputBox.Text = translatorK2J.Translate(chatInputBox.Text);
        }

        private async void SendButton_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                await connector.SendChat(chatInputBox.Text);
                chatInputBox.Text = string.Empty;
            }
            catch (Exception exception)
            {
                AppendChatLog("Send message failed : " + exception.Message);
            }
        }

        private void MacroShortcutInputBox_KeyDown(object sender, KeyEventArgs e)
        {
            if (macroShortcutEditing)
            {
                if (e.Key == Key.Escape)
                {
                    macroShortcutInputBox.Text = macroKey.ToString();
                    macroShortcutEditing = false;
                }
                else
                {
                    macroKey = e.Key;
                    macroShortcutInputBox.Text = macroKey.ToString();
                    macroShortcutEditing = false;
                }
            }
        }

        private void MacroShortcutInputBox_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            macroShortcutEditing = true;
            macroShortcutInputBox.Text = "Press key...";
        }

        private void ProcessRefreshButton_Click(object sender, RoutedEventArgs e)
        {
            UpdateProcessList();
        }

        private void UpdateProcessList()
        {
            Process[] processes = FFXIVConnector.FindFFXIVProcess();

            processList.Items.Clear();
            processList.Tag = processes;

            foreach (Process process in processes)
            {
                processList.Items.Add(string.Format("{0}:{1}", process.ProcessName, process.Id));
            }

            if (!processList.Items.IsEmpty)
            {
                processList.SelectedIndex = 0;
            }
        }

        private async void ProcessSelectButton_Click(object sender, RoutedEventArgs e)
        {
            var processes = processList.Tag as Process[];

            if (processList.SelectedIndex >= processes.Length)
            {
                MessageBox.Show("Process selection index error.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
            else if (processList.SelectedIndex < 0)
            {
                MessageBox.Show("No process selected.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
            else
            {
                try
                {
                    Process selectedProcess = processes[processList.SelectedIndex];

                    AppendChatLog(string.Format("Connecting to process {0}...", selectedProcess.Id));
                    connector = await FFXIVConnector.CreateConnector(selectedProcess, AppendChatLog);
                    AppendChatLog("Conntected.");

                    SetWindowState(MainWindowState.MacroNotSelected);
                }
                catch (Exception exception)
                {
                    AppendChatLog("Process connection failed: " + exception.Message);
                }
            }
        }

        private void SetWindowState(MainWindowState state)
        {
            switch (state)
            {
                case MainWindowState.ProcessDisconnected:
                    sendButton.IsEnabled = false;
                    setMacroButton.IsEnabled = false;
                    detachButton.IsEnabled = false;
                    break;
                case MainWindowState.MacroNotSelected:
                    sendButton.IsEnabled = false;
                    setMacroButton.IsEnabled = true;
                    detachButton.IsEnabled = true;
                    break;
                case MainWindowState.AllAvailable:
                    sendButton.IsEnabled = true;
                    setMacroButton.IsEnabled = true;
                    detachButton.IsEnabled = true;
                    break;
                default:
                    throw new ArgumentException("Invalid state : " + state);
            }
        }

        private async void SetMacroButton_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                int macroNumber = await connector.SetMacroToUse(macroNameTextBox.Text);
                connector.SetMacroKey(macroKey);
                SetWindowState(MainWindowState.AllAvailable);
                AppendChatLog(string.Format("Selected macro #{0}.", macroNumber));
            }
            catch (Exception exception)
            {
                AppendChatLog("Macro selection failed : " + exception.Message);
            }
        }

        public void AppendChatLog(string log)
        {
            if (Dispatcher.CheckAccess())
            {
                var now = DateTime.Now;
                int newItemIndex = chatListView.Items.Add(string.Format("[{0}:{1}] {2}", now.Hour, now.Minute, log));

                if (newItemIndex >= 0)
                {
                    chatListView.ScrollIntoView(chatListView.Items[newItemIndex]);
                }

                try
                {
                    Clipboard.SetDataObject(log);
                }
                catch (Exception e)
                {
                    chatListView.Items.Add(string.Format("[{0}:{1}] {2}", now.Hour, now.Minute, e.ToString()));
                }
            }
            else
            {
                Dispatcher.InvokeAsync(() => AppendChatLog(log));
            }
        }

        private void DetachButton_Click(object sender, RoutedEventArgs e)
        {
            if (connector != null)
            {
                connector.Dispose();
                connector = null;
            }
            SetWindowState(MainWindowState.ProcessDisconnected);
        }
    }
}

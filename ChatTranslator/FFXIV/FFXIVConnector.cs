using ChatTranslator.FFXIV.Exceptions;
using System;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Input;

namespace ChatTranslator.FFXIV
{
    public class FFXIVConnector : IDisposable
    {
        public delegate void ScenarioStringReceiveHandler(string scenarioString);

        protected Process ffxivProcess;
        protected IntPtr macroPosition;
        protected IntPtr ffxivWindowHandle;
        protected int macroNumber = -1;
        protected Key macroKey = Key.None;
        protected Thread scenarioReadThread;
        protected ScenarioStringReceiveHandler scenarioHandler;
        protected bool debuggerSet = true;

        public static Process[] FindFFXIVProcess()
        {
            return Process.GetProcessesByName("ffxiv_dx11");
        }

        public async static Task<FFXIVConnector> CreateConnector(Process ffxivProcess, ScenarioStringReceiveHandler scenarioHandler)
        {
            var instance = new FFXIVConnector(ffxivProcess);
            instance.scenarioHandler = scenarioHandler;
            await Task.Factory.StartNew(instance.Initialize);
            return instance;
        }

        protected FFXIVConnector(Process ffxivProcess)
        {
            this.ffxivProcess = ffxivProcess;
            macroPosition = IntPtr.Zero;
        }

        protected void Initialize()
        {
            SearchMemoryPositions();
            ffxivWindowHandle = NativeOperations.GetFFXIVWindowHandle(ffxivProcess);
            scenarioReadThread = new Thread(ScenarioReaderMain);
            scenarioReadThread.Start();
        }

        protected void ScenarioReaderMain()
        {
            NativeOperations.StartReading(ffxivProcess);
            debuggerSet = true;

            try
            {
                while (!ffxivProcess.HasExited && !disposedValue)
                {
                    string scenarioString = NativeOperations.WaitAndReadScenarioString(ffxivProcess);
                    scenarioHandler(scenarioString);
                }
            }
            catch (Exception)
            {
            }
            finally
            {
                UnloadDebugger();
            }
        }

        protected void UnloadDebugger()
        {
            if (debuggerSet)
            {
                NativeOperations.StopReading(ffxivProcess);
                debuggerSet = false;
            }
        }

        protected void SearchMemoryPositions()
        {
            macroPosition = NativeOperations.SearchMacroLocation(ffxivProcess);
        }

        public async Task<int> SetMacroToUse(string macroName)
        {
            string getMacroName(object macroNumber) => NativeOperations.GetMacroName(ffxivProcess, macroPosition, (int) macroNumber);
            var nameGettingTasks = new Task<string>[100];

            for (int i = 0; i < 100; i++)
            {
                object capturedIndex = i;
                nameGettingTasks[i] = Task.Factory.StartNew(getMacroName, capturedIndex, CancellationToken.None, TaskCreationOptions.DenyChildAttach, TaskScheduler.Default);
            }

            await Task.WhenAll(nameGettingTasks);


            for (int i = 0; i < nameGettingTasks.Length; i++)
            {
                if (nameGettingTasks[i].Result == macroName)
                {
                    macroNumber = i;
                    return i;
                }
            }

            throw new MacroNotFoundException(string.Format("Failed to find macro named {0}.", macroName));
        }

        public void SetMacroKey(Key key)
        {
            macroKey = key;
        }

        public async Task SendChat(string chat)
        {
            await Task.Factory.StartNew(() => SetMacro(chat));
            await PressMacroKey();
        }

        protected void SetMacro(string content)
        {
            NativeOperations.WriteMacro(ffxivProcess, macroPosition, macroNumber, 0, content);
        }

        protected async Task PressMacroKey()
        {
            if (macroKey != Key.None)
            {
                NativeOperations.SendKeyDownEvent(ffxivWindowHandle, macroKey);
                await Task.Delay(200);
                NativeOperations.SendKeyUpEvent(ffxivWindowHandle, macroKey);
            }
        }

        #region IDisposable Support
        private bool disposedValue = false;

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (disposing)
                {
                }

                disposedValue = true;
            }
        }

        public void Dispose()
        {
            Dispose(true);
        }
        #endregion
    }
}

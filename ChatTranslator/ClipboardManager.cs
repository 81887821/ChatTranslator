using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Threading;

namespace ChatTranslator
{
    internal class ClipboardManager
    {
        private readonly Queue<string> clipboardWriteQueue;
        private TimeSpan currentWriteInterval;

        internal ClipboardManager(Dispatcher dispatcher)
        {
            clipboardWriteQueue = new Queue<string>();
            currentWriteInterval = TimeSpan.FromSeconds(2);
            dispatcher.Invoke(ClipboardWriteRoutine);
        }

        internal void WriteToClipboard(string message)
        {
            lock (clipboardWriteQueue)
            {
                clipboardWriteQueue.Enqueue(message);
            }
        }

        internal async Task ClipboardWriteRoutine()
        {
            string lastWrittenString = string.Empty;
            DateTime lastWrittenTime = DateTime.Now;
            string currentString;
            int numberOfRemainStrings;

            while (true)
            {
                lock (clipboardWriteQueue)
                {
                    if (clipboardWriteQueue.Count <= 0)
                    {
                        currentString = null;
                    }
                    else
                    {
                        currentString = clipboardWriteQueue.Dequeue();
                    }
                    numberOfRemainStrings = clipboardWriteQueue.Count;
                }

                if (currentString != null)
                {
                    if (currentString == lastWrittenString)
                    {
                        continue;
                    }

                    await WaitForNextWrite(lastWrittenTime, numberOfRemainStrings);

                    try
                    {
                        Clipboard.SetDataObject(currentString);
                    }
                    catch (Exception)
                    {
                        currentWriteInterval += TimeSpan.FromMilliseconds(100);
                    }

                    lastWrittenString = currentString;
                    lastWrittenTime = DateTime.Now;
                }

                if (numberOfRemainStrings <= 0)
                {
                    await Task.Delay(1000);
                }
            }
        }

        internal async Task WaitForNextWrite(DateTime lastWrittenTime, int numberOfRemainStrings)
        {
            await SleepUntil(lastWrittenTime + currentWriteInterval);

            if (numberOfRemainStrings >= 10)
            {
                currentWriteInterval = TimeSpan.FromMilliseconds(0.5);
            }
            else if (numberOfRemainStrings <= 0)
            {
                currentWriteInterval = TimeSpan.FromSeconds(2);
            }
        }

        internal async Task SleepUntil(DateTime time)
        {
            DateTime now = DateTime.Now;

            if (now >= time)
            {
                return;
            }
            else
            {
                await Task.Delay(time - now);
            }
        }
    }
}

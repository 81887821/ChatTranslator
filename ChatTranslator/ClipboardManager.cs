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
        private static readonly TimeSpan INTERVAL_MINIMUM = TimeSpan.FromMilliseconds(100);
        private static readonly TimeSpan INTERVAL_VERY_SHORT = TimeSpan.FromMilliseconds(250);
        private static readonly TimeSpan INTERVAL_SHORT = TimeSpan.FromMilliseconds(500);
        private static readonly TimeSpan INTERVAL_NORMAL = TimeSpan.FromMilliseconds(2000);

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

                    await WaitForNextWrite(lastWrittenTime, lastWrittenString.Length);
                    SetNextWriteInterval(numberOfRemainStrings);

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

        internal async Task WaitForNextWrite(DateTime lastWrittenTime, int lastWrittenStringLength)
        {
            TimeSpan intervalByLength = TimeSpan.FromMilliseconds(150.0 + lastWrittenStringLength * 100);
            TimeSpan interval = Min(currentWriteInterval, intervalByLength);
            await SleepUntil(lastWrittenTime + interval);
        }

        private static TimeSpan Min(TimeSpan left, TimeSpan right)
        {
            return left > right ? right : left;
        }

        internal void SetNextWriteInterval(int numberOfRemainStrings)
        {
            if (numberOfRemainStrings >= 20)
            {
                currentWriteInterval = INTERVAL_MINIMUM;
            }
            else if (numberOfRemainStrings >= 10)
            {
                currentWriteInterval = Min(currentWriteInterval, INTERVAL_VERY_SHORT);
            }
            else if (numberOfRemainStrings >= 5)
            {
                currentWriteInterval = Min(currentWriteInterval, INTERVAL_SHORT);
            }
            else if (numberOfRemainStrings <= 0)
            {
                currentWriteInterval = INTERVAL_NORMAL;
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

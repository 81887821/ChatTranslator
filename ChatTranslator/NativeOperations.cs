using ChatTranslator.Exceptions;
using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows.Input;

namespace ChatTranslator
{
    public static class NativeOperations
    {
        private const string DLL_NAME = "NativeOperations.dll";

        private enum DllErrorCode : int
        {
            Success = 0,
            InternalError = -1,
            MemoryPointNotFound = -2,
            MultipleMemoryPointsFound = -3,
            InsufficientBufferSize = -4,
            InvalidMacroNumber = -5,
            InvalidMacroLine = -6,
            InsufficientMacroLineCapacity = -7,
            NoWindowFound = -8,
        }

        public static IntPtr SearchMacroLocation(Process ffxivProcess)
        {
            DllErrorCode errorCode = SearchMacroLocation(ffxivProcess.Handle, out IntPtr macroBasePosition);

            switch (errorCode)
            {
                case DllErrorCode.Success:
                    return macroBasePosition;
                case DllErrorCode.InternalError:
                    throw new DllInnerException("SearchMacroLocation failed with unknown reason.");
                case DllErrorCode.MemoryPointNotFound:
                    throw new MemoryPatternNotFoundException("Cannot find macro location.");
                default:
                    throw new DllInnerException("SearchMacroLocation returned unexpected error code : " + errorCode);
            }
        }

        public static unsafe string GetMacroName(Process ffxivProcess, IntPtr macroPosition, int macroNumber)
        {
            int bufferSize = 1024;
            byte[] buffer = new byte[bufferSize];
            IntPtr processHandle = ffxivProcess.Handle;

            while (true)
            {
                fixed (byte* bufferPointer = buffer)
                {
                    DllErrorCode errorCode = GetMacroName(processHandle, macroPosition, macroNumber, bufferPointer, bufferSize, out int macroNameLength);

                    switch (errorCode)
                    {
                        case DllErrorCode.Success:
                            return Encoding.UTF8.GetString(bufferPointer, macroNameLength);
                        case DllErrorCode.InsufficientBufferSize:
                            if (bufferSize > macroNameLength)
                            {
                                throw new DllInnerException(string.Format("InsufficientBufferSize returned though bufferSize({0}) is lager than macroNameLength({1}).", bufferSize, macroNameLength));
                            }
                            else
                            {
                                bufferSize = macroNameLength;
                                buffer = new byte[bufferSize];
                                continue;
                            }
                        case DllErrorCode.InvalidMacroNumber:
                            throw new InvalidMacroNumberException(macroNumber);
                        case DllErrorCode.InternalError:
                            throw new DllInnerException("GetMacroName failed with unknown reason.");
                        default:
                            throw new DllInnerException("GetMacroName returned unexpected error code : " + errorCode);
                    }
                }
            }
        }

        public static int GetMacroLineCapacity(Process ffxivProcess, IntPtr macroPosition, int macroNumber, int macroLine)
        {
            DllErrorCode errorCode = GetMacroLineCapacity(ffxivProcess.Handle, macroPosition, macroNumber, macroLine, out int capacity);

            switch (errorCode)
            {
                case DllErrorCode.Success:
                    return capacity;
                case DllErrorCode.InternalError:
                    throw new DllInnerException("GetMacroLineCapacity failed with unknown reason.");
                case DllErrorCode.InvalidMacroNumber:
                    throw new InvalidMacroNumberException(macroNumber);
                case DllErrorCode.InvalidMacroLine:
                    throw new InvalidMacroLineException(macroLine);
                default:
                    throw new DllInnerException("GetMacroLineCapacity returned unexpected error code : " + errorCode);
            }
        }

        public static unsafe void WriteMacro(Process ffxivProcess, IntPtr macroPosition, int macroNumber, int macroLine, string content)
        {
            byte[] contentBytes = new byte[Encoding.UTF8.GetByteCount(content) + 1];

            Encoding.UTF8.GetBytes(content, 0, content.Length, contentBytes, 0);
            contentBytes[contentBytes.Length - 1] = 0;

            fixed (byte* contentBytesPointer = contentBytes)
            {
                DllErrorCode errorCode = WriteMacro(ffxivProcess.Handle, macroPosition, macroNumber, macroLine, contentBytesPointer, contentBytes.Length);

                switch (errorCode)
                {
                    case DllErrorCode.Success:
                        return;
                    case DllErrorCode.InternalError:
                        throw new DllInnerException("WriteMacro failed with unknown reason.");
                    case DllErrorCode.InvalidMacroNumber:
                        throw new InvalidMacroNumberException(macroNumber);
                    case DllErrorCode.InvalidMacroLine:
                        throw new InvalidMacroLineException(macroLine);
                    case DllErrorCode.InsufficientMacroLineCapacity:
                        throw new InsufficientMacroLineCapacityException();
                    default:
                        throw new DllInnerException("WriteMacro returned unexpected error code : " + errorCode);
                }
            }
        }

        public static IntPtr GetFFXIVWindowHandle(Process ffxivProcess)
        {
            DllErrorCode errorCode = GetFFXIVWindowHandle(ffxivProcess.Handle, out IntPtr windowHandle);

            switch (errorCode)
            {
                case DllErrorCode.Success:
                    return windowHandle;
                case DllErrorCode.InternalError:
                    throw new DllInnerException("GetWindowHandle failed with unknown reason.");
                case DllErrorCode.NoWindowFound:
                    throw new WindowNotFoundException();
                default:
                    throw new DllInnerException("GetWindowHandle returned unexpected error code : " + errorCode);
            }
        }

        public static void SendKeyDownEvent(IntPtr windowHandle, Key key)
        {
            SendKeyEvent(windowHandle, key, true);
        }

        public static void SendKeyUpEvent(IntPtr windowHandle, Key key)
        {
            SendKeyEvent(windowHandle, key, false);
        }

        private static void SendKeyEvent(IntPtr windowHandle, Key key, bool keyDownEvent)
        {
            int virtualKeyCode = KeyInterop.VirtualKeyFromKey(key);
            DllErrorCode errorCode = SendKeyEvent(windowHandle, virtualKeyCode, keyDownEvent);

            switch (errorCode)
            {
                case DllErrorCode.Success:
                    return;
                case DllErrorCode.InternalError:
                    throw new DllInnerException("SendKeyEvent failed with unknown reason.");
                default:
                    throw new DllInnerException("SendKeyEvent returned unexpected error code : " + errorCode);
            }
        }

        public static void StartReading(Process ffxivProcess)
        {
            DllErrorCode errorCode = StartReading(ffxivProcess.Handle);

            switch (errorCode)
            {
                case DllErrorCode.Success:
                    return;
                case DllErrorCode.InternalError:
                    throw new DllInnerException("StartReading failed with unknown reason.");
                default:
                    throw new DllInnerException("StartReading returned unexpected error code : " + errorCode);
            }
        }

        public static void StopReading(Process ffxivProcess)
        {
            DllErrorCode errorCode = StopReading(ffxivProcess.Handle);

            switch (errorCode)
            {
                case DllErrorCode.Success:
                    return;
                case DllErrorCode.InternalError:
                    throw new DllInnerException("StopReading failed with unknown reason.");
                default:
                    throw new DllInnerException("StopReading returned unexpected error code : " + errorCode);
            }
        }

        public static unsafe string WaitAndReadScenarioString(Process ffxivProcess)
        {
            int bufferSize = 1024;
            byte[] buffer = new byte[bufferSize];
            IntPtr processHandle = ffxivProcess.Handle;

            while (true)
            {
                fixed (byte* bufferPointer = buffer)
                {
                    DllErrorCode errorCode = WaitAndReadScenarioString(processHandle, bufferPointer, bufferSize, out int stringLength);

                    switch (errorCode)
                    {
                        case DllErrorCode.Success:
                            return Encoding.UTF8.GetString(bufferPointer, stringLength);
                        case DllErrorCode.InsufficientBufferSize:
                            if (bufferSize > stringLength)
                            {
                                throw new DllInnerException(string.Format("InsufficientBufferSize returned though bufferSize({0}) is lager than macroNameLength({1}).", bufferSize, stringLength));
                            }
                            else
                            {
                                bufferSize = stringLength;
                                buffer = new byte[bufferSize];
                                continue;
                            }
                        case DllErrorCode.InternalError:
                            throw new DllInnerException("WaitAndReadScenarioString failed with unknown reason.");
                        default:
                            throw new DllInnerException("WaitAndReadScenarioString returned unexpected error code : " + errorCode);
                    }
                }
            }
        }

        [DllImport(DLL_NAME)]
        private static extern DllErrorCode SearchMacroLocation(IntPtr ffxivProcessHandle, out IntPtr macroPosition);
        [DllImport(DLL_NAME)]
        private static unsafe extern DllErrorCode GetMacroName(IntPtr ffxivProcessHandle, IntPtr macroPosition, int macroNumber, void* buffer, int bufferCapacity, out int macroNameLength);
        [DllImport(DLL_NAME)]
        private static extern DllErrorCode GetMacroLineCapacity(IntPtr ffxivProcessHandle, IntPtr macroPosition, int macroNumber, int macroLine, out int capacity);
        [DllImport(DLL_NAME)]
        private static unsafe extern DllErrorCode WriteMacro(IntPtr ffxivProcessHandle, IntPtr macroPosition, int macroNumber, int macroLine, void* content, int contentSize);
        [DllImport(DLL_NAME)]
        private static extern DllErrorCode GetFFXIVWindowHandle(IntPtr ffxivProcessHandle, out IntPtr windowHandle);
        [DllImport(DLL_NAME)]
        private static extern DllErrorCode SendKeyEvent(IntPtr windowHandle, int virtualKeyCode, bool keyDownEvent);
        [DllImport(DLL_NAME)]
        private static extern DllErrorCode StartReading(IntPtr ffxivProcessHandle);
        [DllImport(DLL_NAME)]
        private static extern DllErrorCode StopReading(IntPtr ffxivProcessHandle);
        [DllImport(DLL_NAME)]
        private static unsafe extern DllErrorCode WaitAndReadScenarioString(IntPtr ffxivProcessHandle, void* buffer, int bufferCapacity, out int stringLength);
    }
}

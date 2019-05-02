using System;

namespace ChatTranslator.Exceptions
{
    public class NativeOperationException : Exception
    {
        public NativeOperationException()
        {
        }

        public NativeOperationException(string message) : base(message)
        {
        }

        public NativeOperationException(string message, Exception innerException) : base(message, innerException)
        {
        }
    }
}

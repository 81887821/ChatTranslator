using System;

namespace ChatTranslator.Exceptions
{
    public class DllInnerException : NativeOperationException
    {
        public DllInnerException(string errorMessage) : base(errorMessage)
        {
        }
    }
}

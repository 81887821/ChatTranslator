namespace ChatTranslator.Exceptions
{
    public class MemoryPatternNotFoundException : NativeOperationException
    {
        public MemoryPatternNotFoundException(string errorMessage) : base(errorMessage)
        {
        }
    }
}

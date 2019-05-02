namespace ChatTranslator.Exceptions
{
    public class InvalidMacroLineException : NativeOperationException
    {
        public InvalidMacroLineException(int macroLine) : base(MakeMessage(macroLine))
        {
        }

        protected static string MakeMessage(int macroLine)
        {
            return string.Format("{0} is not a valid macro line.", macroLine);
        }
    }
}

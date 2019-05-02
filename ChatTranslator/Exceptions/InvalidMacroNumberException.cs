namespace ChatTranslator.Exceptions
{
    public class InvalidMacroNumberException : NativeOperationException
    {
        public InvalidMacroNumberException(int macroNumber) : base(MakeMessage(macroNumber))
        {
        }

        protected static string MakeMessage(int macroNumber)
        {
            return string.Format("{0} is not a valid macro number.", macroNumber);
        }
    }
}

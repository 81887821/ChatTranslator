using System;

namespace ChatTranslator.FFXIV.Exceptions
{
    public class MacroNotFoundException : Exception
    {
        public MacroNotFoundException(string message) : base(message)
        {
        }
    }
}

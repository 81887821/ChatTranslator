namespace ChatTranslator.Translator
{
    class DummyTranslator : ITranslator
    {
        public string Translate(string originalString)
        {
            return originalString;
        }
    }
}

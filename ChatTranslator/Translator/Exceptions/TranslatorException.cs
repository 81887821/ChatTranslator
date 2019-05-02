﻿using System;

namespace ChatTranslator.Translator.Exceptions
{
    public class TranslatorException : Exception
    {
        public TranslatorException()
        {
        }

        public TranslatorException(string message) : base(message)
        {
        }

        public TranslatorException(string message, Exception innerException) : base(message, innerException)
        {
        }
    }
}

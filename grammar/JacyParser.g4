parser grammar JacyParser;

options { tokenVocab = JacyLexer; }

jacyFile: ifStmt* EOF;

ifStmt: IF NL*;

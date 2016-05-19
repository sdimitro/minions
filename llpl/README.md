# llpl
Little Lisper's Probabilities Library

Rudimentary probabilities library for Common Lisp. Code will be
refactored as I become more acquainted to the conventions of
the language.

### What is implemented?

Currently simple single discrete distributions can be represented
and queried with simple questions. There is support for
representation of discrete joint distributions and elementary
querying for them.

### What is next?

I'll keep working on discrete distributions because
that is what I mostly use at the moment on my classes.
So checking for independencies within joint distributions
should be next. After that I'm planning to implement
support for conditional probability distributions
and finally generation of tables through the Bayes
Theorem.

It may be too soon for long term plans but one of
the top goals that I have for this project is to
encorporate a framework for working with Probabilistic
Graphical Models.

### Other Notes

Issues, feedback and forks are welcome. Pull requests are
welcome to but I can't promise that your code will be merged.
The reason is that I'm still learning Lisp and I want to
be completely sure that whatever I merge, I understand
fully.

If you are a seasoned/reasoned Lisper, I would really appreciate it
if you could spend 2 mins to go through my code and shared any
feedback you have as an issue.

### Obligatory xkcd Comic:

![xkcd - Seashell](https://imgs.xkcd.com/comics/seashell.png)

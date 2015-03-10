#RShell - Current Bugs

Connector Logic: fail in `pwd; echo cat && exit`
* does not do `exit`
* suspect: saving return status not optimal for multiple arguments w/ semicolon

Connector Logic: fail in `echo infinite||echo blade&&echo pie`
* does not omit blade
* suspect: saving and checking retStatus when commands skipped with `||`


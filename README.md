The idea is to build a scaffolding for small local models to try to improve their performance.
The plan is to take a prompt, strip it for keywords and eventually context, compare it again an 
interchangable knowledge base, any matching records in the knowledge base are return. The prompt 
and the returned records are fused and sent to the model. The response is then printed to the 
screen.

PROMPT -> KEYWORDS -> RETRIVE DATABASE MATCHES AS CONTEXT -> RECONSTRUCT PROMPT -> MODEL -> RESPONSE 

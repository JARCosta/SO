// criar threads
    // criar file x
    // escrever y
    // fechar file x







/*
thread 1:
    create(x)
    write(y)
    close(x)
thread 2:
    esperou q o file fosse criado
    verificou q o file ja foi criado
    abriu o file ou exit(ERROR)
    se abriu, escreveu y
thread 3:
    esperou q o file fosse criado
    esperou q thread 2 verificasse que file ja tinha sido criado
    verificou q file ja tinha sido criado
    abriu file ou exit(ERROR)
    se abriu, escreveu y
thread 4:
    .
    .
    .
exit(0)
wrong answer
*/
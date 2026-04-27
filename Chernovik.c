while ((len = recv(my_sock, buff, 1024, 0)) > 0) {
    if (len < 1024)
        buff[len] = '\0';

    cout << "Received: " << buff << endl;
    
    // ПЕРЕВОРАЧИВАЕМ СТРОКУ
    string original(buff);
    string reversed(original.rbegin(), original.rend());
    
    cout << "Sending reversed: " << reversed << endl;
    send(my_sock, reversed.c_str(), reversed.size(), 0);
}

u32 str8ncmp(String8 s1, String8 s2, u32 n)
{
    u32 i = 0;
    while (i < s1.size && i < s2.size && i < n && s1.str[i] == s2.str[i])
    {
        i += 1;
    }
    if (i == n)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
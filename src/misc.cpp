
bool compareTicks(unsigned int t1, unsigned int t2)
{
    if(t1 > t2 && t1-t2 < 2147483648u) return true;
    if(t1 < t2 && t2-t1 > 2147483648u) return true;
    return false;
}


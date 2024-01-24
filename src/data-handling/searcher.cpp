#include <searcher.h>

#include <importer.h>

using namespace std;

int ConnectionSearcher::binarySearch(int departureTime)
{
    int left = 0;
    int right = Importer::connections.size() - 1;
    int mid = 0;

    while (left <= right)
    {
        mid = (left + right) / 2;

        if (Importer::connections[mid].departureTime >= departureTime && (mid == 0 || Importer::connections[mid - 1].departureTime < departureTime))
        {
            return mid;
        }
        else if (Importer::connections[mid].departureTime < departureTime)
        {
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }

    return mid;
}
var relatedTitles = new Array();
var relatedTitlesNum = 0;
var relatedUrls = new Array();

function related_results_labels_thumbs(json)
{
  for (var i = 0; i < json.feed.entry.length; ++i)
  {
    var entry = json.feed.entry[i];
    relatedTitles[relatedTitlesNum] = entry.title.$t;

    //if(relatedTitles[relatedTitlesNum].length>35)
    //{
      //relatedTitles[relatedTitlesNum]=relatedTitles[relatedTitlesNum].substring(0, 35)+"...";
    //}

    for (var k = 0; k < entry.link.length; ++k)
    {
      if (entry.link[k].rel == 'alternate')
      {
        relatedUrls[relatedTitlesNum] = entry.link[k].href;
        ++relatedTitlesNum;
      }
    }
  }
}

function removeDuplicatesInRelatedPosts()
{
  var tmp = new Array(0);
  var tmp2 = new Array(0);
  var tmp3 = new Array(0);
  for(var i = 0; i < relatedUrls.length; ++i)
  {
    if(!isExist(tmp, relatedUrls[i]))
    {
      tmp.length += 1;
      tmp[tmp.length - 1] = relatedUrls[i];
      tmp2.length += 1;
      tmp3.length += 1;
      tmp2[tmp2.length - 1] = relatedTitles[i];
    }
  }

  relatedTitles = tmp2;
  relatedUrls = tmp;
}

function isExist(a, e)
{
  for(var j = 0; j < a.length; j++) 
  {
    if (a[j]==e)
    {
      return true;
    }
  }
  return false;
}

function printRelatedPosts()
{
  for(var i = 0; i < relatedUrls.length; ++i)
  {
    if((relatedUrls[i]==currentposturl)||(!(relatedTitles[i])))
    {
      relatedUrls.splice(i,1);
      relatedTitles.splice(i,1);
    }
  }

  var r = Math.floor((relatedTitles.length - 1) * Math.random());
  var i = 0;

  if(relatedTitles.length > 0)
  {
    document.write('<h2>');
    document.write(relatedpoststitle);
    document.write('</h2>');
  }

  while (i < relatedTitles.length && i < 20 && i < maxresults)
  {
    document.write('<a ');
    document.write('href="');
    document.write(relatedUrls[r]);
    document.write('">');
    document.write(relatedTitles[r]);
    document.write('</a>');
    document.write('<br>');

    if (r < relatedTitles.length - 1)
    {
      r++;
    }
    else
    {
      r = 0;
    }
    i++;
  }
  document.write('<br>');
}


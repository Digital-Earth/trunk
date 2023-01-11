app.factory('styleOptions', function () {
    return {
        icons: [
            {
                'name': 'triangle_up',
                'dataUrl': 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAA0AAAAMCAMAAACOacfrAAAABGdBTUEAALGPC/xhBQAAACBjSFJNAAB6JgAAgIQAAPoAAACA6AAAdTAAAOpgAAA6mAAAF3CculE8AAAAVFBMVEUAAAD///////////////////////////////////////////////////////+ampoHBwfr6+sQEBAAAABnZ2fPz88CAgL+/v49PT2oqKggICAyMjJXnW4kAAAADnRSTlMAKbz5+7Ncxpcbo+DOkRp2GvkAAAABYktHRACIBR1IAAAACXBIWXMAAAsSAAALEgHS3X78AAAAX0lEQVQI10XHVxKEMBTEwDE52CI8Mve/J7B4C32ppTuXpIlTzGVA9mdO13fkLwqG0caB4qeSycwmygcV83JrmamkmnWzp22lVsNubzuNWo6og1aeM+rEK/AVpODj+6AL51MHTXcjzmEAAAAASUVORK5CYII=',
                'oldDataUrl': 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAAAXNSR0IArs4c6QAAAAZiS0dEAP8A/wD/oL2nkwAAAAlwSFlzAAALEwAACxMBAJqcGAAAAAd0SU1FB94MCBcSAms3thsAAAAdaVRYdENvbW1lbnQAAAAAAENyZWF0ZWQgd2l0aCBHSU1QZC5lBwAABQNJREFUeNrtms1rG0cYxn9aybbs5kNu0mToB1iHgNVe1ocEtTn00rQmBVeQQp1LemgOgqagQ0G9iULBFAw6+h8wvfXSY1vo1T4UOZeKgkGG0uJAHMvQxuu6tnrIO/ZmOyutpF1JSfeBxdbu7Oy88z7zvs98QIwYMWLEiBEjxv8UiQF/z5LL77st4Fiu564DksAEMAWMGb7dAg6Bx8ABcPQ8dUASmARelGtS7rlxBOwDj+TaH0QnpAZE+wkxXMnfCbnvxrF4flx+PwScqIdDakDePwNcEOOn5J5pCGhWHIrxh3K/9ax2gPb+WeC8i/qvAm95yt4HfpUy54E/pRP+inIopCKOL0nxeMbj+dvAVU/5d4B74m3NmAPgnyiHQjJi6k+JIRfFqBRgAx8ZyqfF65vCnKQrLhxE1QFWxNTPSAeccXX27TbvLQDT8r4eCmd9guZIMyAljb8s3tdj/zrwXpv30mLofVeQ1OkxEhZYAwp8lqS3OwHev+Figa4nMhZYEY59b+C7Ifc6YQz4pE1dI90Bfl6bBm55C+fzeUqlkqmeq0B2ECwIWwqPibGvyfifkgZ/DNz0Fq7VaiilyGazOI7jffwL8KWM+8fAA+A3YFdSY2vUGODnrUtC/6dQKBSwbRulFJVKxVTf68C1NjFlpBiQkMhv8v7nXtGTTqep1+vMzMwA4DgO2WyW7e1tb71bQFlY4Mj84I8w5wlWiN435e2sQfFRLBZPjNcd4sOCGeD9Drpi6AywJH9fBF6WvzqffwVccRfOZDLU63WUUv+paG5ujo2NDe/tJvCZSxbvSix4ILHheNgMcM/2Mi7vX/MaD1CpVIzG62cGZIAP2swthqoELddCxyWhv175+dSb95VSrK6ukkqZ52Czs7Osr6+zubnpfXQF+ElYkHDFBKffjGCF0IF+omfGW7harZJOp9tWuLS05JdeF6IQR1af75rS3rirsSewbZvFxcWOlbYpdzMKcWSF7H2AD02S18ezRrRhyq2wWWCF7P1pk+IrFArMz88HrlwpRbFY9JPIb4TJgl7ToJ/kvWtSfbVaDdu2u/pAs9kkl8t1Ekd9S2QrRO9nTcYXi8Wujdd6oVwu+4mj0CRyoofyXUneRqPhm/c7wXEccrkcW1tbJnF0T9JiXxLZ6sH7JslrmyRvqVTq2XjdgdVq1U8cvRuGRE50abyf5P3am/eVUjQajY55PwjaSOQveLKL1LNE7oYBfpL3ukn0lMvlUIwHWFlZ8WPBQr8S2eoh8HVc57Nt2y+N9YR8Pk+hUIhk/dDqwvuB1/kqlUpo3u9QZ9/rh8kuxv4F4CVONzimgRKnm5kn3lpeXg59pVUpxc7ODmtra95HrwA/A3ueiZLeXW71ywAd+c95enYBeCHgeA0FbeLKHQ8LzgXVBUH2BhNCtUlODzYY1/l0xB4C9PrhuqGtocSAhOd6Wz4wSnjT0M7QgqA+u6P36n+UPDwqOAS+c7XvOOicwApo/CFP78/tAt+OUAf8ADQ43U3e5/RwRd8doGddj8TruhO+l5nZsNEEvnEZ35S2BlKDQaVwUiL+ZUk70xJAEyPCgJZLDv8ucjjQyZKgJ0SOhVZ7ogPGCXmHpk+427cn/weaC6S66OEjnpzgeCi/J4n2hEk30GcIdqSNRwQMgt1SWC95TwoLRmkI/O0K1IEPVfViQKfjrsPshIEes40RI0aMGDFixIgR4xnGvw2eP41tsJssAAAAAElFTkSuQmCC',
            },
            {
                'name': 'triangle_down',
                'dataUrl': 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAA0AAAAMCAMAAACOacfrAAAABGdBTUEAALGPC/xhBQAAACBjSFJNAAB6JgAAgIQAAPoAAACA6AAAdTAAAOpgAAA6mAAAF3CculE8AAAAVFBMVEUAAAD///////////////////////////////////////////////////////8yMjIAAAAgICCoqKj+/v49PT3Pz88CAgJnZ2fr6+sQEBCampoHBwd405rtAAAADnRSTlMAkc7goxuXxlyzKfu8+d9Qi1AAAAABYktHRACIBR1IAAAACXBIWXMAAAsSAAALEgHS3X78AAAAXklEQVQI10XNRxaAIADE0AF7C/Z6/3uKoM/s/iqSscSskQx/RhbXxxxWCcOrgUQp46uRVBnTHDBPZFLOsnqsC7l8BZvXRvFAJfvRHztlkCrO66SKUN34c1PrY9u1ATdojgbBioI0xgAAAABJRU5ErkJggg==',
                'oldDataUrl': 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAAAXNSR0IArs4c6QAAAAZiS0dEAP8A/wD/oL2nkwAAAAlwSFlzAAALEwAACxMBAJqcGAAAAAd0SU1FB94MCBcSHJE4i3gAAAAdaVRYdENvbW1lbnQAAAAAAENyZWF0ZWQgd2l0aCBHSU1QZC5lBwAABHlJREFUeNrtms9rG0cUxz+SLP9KSlXstoISqG6GHioTCr702IObk2nooeD27KOhUAS9+CjowUddCr4FWgJqIIXmD6hNfPDeHKipC6VYaeVUsdNoncjeHvI2TNe7ys7uzkrQeTBgvLuaed/3fTPvOzNgzZo1a9asWbNm7f9phQTfFKUVxsgPD7iQZhSAEjAFzACTYwKCBzwD+sAZcK7z8YRm5KeAOWkzAsio7VycP5bm6jBhQoMpJeAqMA+8KQAUxwCACwGgIAx4LqzwsgSgKA6/Lu2KfDsuKVCUNHgiDPgnbipMaFC/ItS/KmwoAF8CH4zQ+V+ArxV2zgkLBnFTIU4Ol4DXgLeF/n7uvwd8NuLozwEnwK8SqJIyJ5zFAaAYM//L4viUfDMJfD4mS+AnwBsKU2dkvIW4uR13EvTX/gLwEfDumABQERAKgXGSJQBeoP0xZgXdUcgYMwPAk6WlrywxDrAbfLFardLv9/E8z0hbWloKG18PuBcxzkwA8NfZE+CpsrzcDr7Y6XRotVpGQtxut9nZ2Ql7dEscPpfxnch4L+Lmdhwry0RzTVaDWQHv0jJYqVTY39+nWq1mCsDi4iKO4wT//RvwlTj7FHgI/A78LaBkNgf46PYCLLgV7KjX69FsNjOPfojzKgujxpepGIpiwRfAx+qL09PTHB4eZsIC13Wp1Wp0Op3Mo6/DgGEo3wl26LpuZixotVphzgN8mzb6SeRwbBYA7O3tUa/XTUR/F/gmbfR1GRCFtie52Au+vLGxkSr6m5ubUdG/nUX0k+4I+Sx4R1hwRWrwG2HlcVIWdDodarUarusai34SBvioP5HNh54iOu6FsaDRaCSKfrPZDHP+ueS+l0X046rBYftvRWBaRIj/7H315YODA+r1OgsLC1rRX11dZTAYBB/9BOwo0T8GuhKQRAAk3dHxq8PHwKnCgruyPKWaCxqNRlT0f5B+ziTyx2mcTwPAMApeKpEdx6Hdbsf6Ycdx2NraCnv0o/QVlYK5AoASidMAC+6HsSAiqsRkSw/4PtDnY52aP8s5IGprbVbmgpLk5YfqC91ul/n5+ShF9zL66+vrYY++Ax7IVtcp8KeA8kxH+mbNgGEsCJXLETP7S1tbWxsmd6P6YpQAoDMXDJPLGnI31bJnysrAW8B1KYlvAp/KfPCfnZpKpeIdHR15QavX617Irs6h/M5N+d3r0k85q4FndbCRSi6blLsmSuFM5bJpuZsXAxLLZdNyN08GaMvl7e1tVlZWjMrdPBmgLZeXl5eNy928CiG1RPYPKyelMCpLAXNJKEXUBLtS9l7w4nzvEfBX2po/LwZoy2XTcjdvBmjLZdNydxQM0JbLJuXuqADQkssm5W4cFYdBFqjiZVZS7j7wMy9OddXoG5G7edcBYQyblZrgmjgddXbvH272ZM1/KMwxCoDpW17BZXFS/lYnSr8NJOKPTE98eaVAcFnsiuNR1+vU6265OJ9HCqhMe9UFy1QXHscdAH8+eNX1lcRXXq1Zs2bNmjVr1qxZ07N/AZnAr7cR6z3pAAAAAElFTkSuQmCC',
            },
            {
                'name': 'hexagon',
                'dataUrl': 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAA4AAAAPBAMAAAAmOuNHAAAABGdBTUEAALGPC/xhBQAAACBjSFJNAAB6JgAAgIQAAPoAAACA6AAAdTAAAOpgAAA6mAAAF3CculE8AAAAMFBMVEUAAAD////////////////////////////////FxcU6Ojra2tpTU1MAAABsbGwFBQUV2cM9AAAACHRSTlMAicZw0Fe+w6OGcXYAAAABYktHRACIBR1IAAAACXBIWXMAAAsSAAALEgHS3X78AAAAQUlEQVQI12NgYGBUEmAAAmaPjhYDBgbWjM5VM9oCGCK6z969s6OVoeL93bt3/7UzdNwFgQ6CNEw9TD/MPJj5UPsAScc4yPeNHf0AAAAASUVORK5CYII=',
                'oldDataUrl': 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAAAXNSR0IArs4c6QAAAAZiS0dEAP8A/wD/oL2nkwAAAAlwSFlzAAALEwAACxMBAJqcGAAAAAd0SU1FB94MCBcUD0PcbSAAAAAdaVRYdENvbW1lbnQAAAAAAENyZWF0ZWQgd2l0aCBHSU1QZC5lBwAAA1VJREFUeNrtmr9PE2EYxz9XqD2IvxDQk8G0To5OJEQSXEyIg4YgMOhQpjJ0gInFhcWVzTCYgGMTB2Pi4qSDm4N/gcUEQ/1dhdgigdehT+OJbenbu3tL6PtN3rzD9e59vp/37b2X53nBysrKysrKqlPlGB4vJq3WuArYl3bsAMSALuAE0APED4ytgF2gBPwG9kyBcAwZTwC9wCngtECI+X63L+Z/AlvAL2DHBAjHoPGz0vfKCjgIYFeMbwFFUyAcw8YTct2p8RdQYnTHJAjHsPHqrPcBY7ISAF4Bn3yrwRgIpw3GbwE3fObxQXhiGoRzBIzTThCtAKgaPxmy8SAgtn0gIgXQJVvYOaAfOBOBcV0QP4CvwDfZSvd0Dema7wc8YFBmPgF0C8w+YAbIAlcaPd/zPLLZLBsbGxSLxUbjJoGbwHngvZjukjETPvjVrXRPdpRQV0AMcIEBMT/g+5hxdGbc8zwWFxeZm5vDdV0A1tbWWFpaYn19XXdFKN9H1BegIH057O2yW0xfBcaBSWAayADP5fNVNWqe56nl5WVVKpVUPa2urqpkMqkOe5a0l7LSpiWecYlvQOINVXHgIjAC3AamgHvAhzCMBwCxLZMwJXGNSJzxKAAMAdeACaH+NGzjLYJ4LfFMSHxDUQIY9QGIzHgLIKoARtsKIGzjtUCEDcDRADAIXJY+DuT+y2goRdRynJohz8gW+Bl4J/1us9tbR8sCsAAsAAvAArAALAALwAKwACwACyCwmszptaxCoXC0AaRSKWZnZ0MHUSgUWFhYIJVKtW2l1EqJvaVBqiqdTqt8Ph8oAbK5uanm5+eV67qNskF5DKXEDiZFMzSRDW4FRJPGq+0+BpKi9dLiD/ibnw8MQtP4d+ARhtLiMSolsEvAMJVKzaSQnwYeBgHRgvHHwF0Zf1LiGZb4enXebTq1QX9p7AKV+mAP/9YErwN3qOQNGyqdTpPJZMjlcqysrFAulw+7pQg8A15QyfdVa4QlKnXBj1RqhFr1waiKo02DaEL1jIdSHI26PB4ExGHG21Ie978TdA5I6IDQMd6WAxJRgTBqPCwAQUCM+e5/Y9p42AB0QRy7Y3K6II7tQclmQHTEUdl6IDrusHQtEEfquLyVlZWVlZVV5+oPm6riGf1qrpsAAAAASUVORK5CYII=',
            },
            {
                'name': 'marker',
                'dataUrl': 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAA0AAAARCAMAAAAxIdauAAAABGdBTUEAALGPC/xhBQAAACBjSFJNAAB6JgAAgIQAAPoAAACA6AAAdTAAAOpgAAA6mAAAF3CculE8AAAAwFBMVEUAAAD////////////////////////////////////////////////////////////////////////////////////////////////////////39/d7e3sdHR0AAAD29vY5OTl8fHwEBAQnJycHBwfOzs7MzMxBQUE9PT0UFBQXFxfx8fHv7+8TExMVFRVfX19tbW0mJiZgYGDS0tIBAQHV1dVjY2NlZWXs7OwWFhbt7e2jo6OlpaX+/v47OzsNDQ2JfsbgAAAAGnRSTlMASKbjnp9H2Ozrr65kY8PCYF6tIB6XyR+28/HVUhMAAAABYktHRACIBR1IAAAACXBIWXMAAAsSAAALEgHS3X78AAAAqUlEQVQI1zWPx0LCABAFHwlFEVRQ2kAeoUWxgtIU0P//K5OAc9q57M5KUiEICYOCcoqcKGZSoj+IHA36lKQyw9gZ8ZCyAkb2eDK2RwQKiTydkUwdEQrsByo82qAL5n565uXVb1yqyrs9WSztD65U43OV71ytqUvXbHLbcpNev2U3T+Xrm0YWU2Wf2oFm3nnHMfbPkftTdovkN6GlM+30g/a/qNPtdTvZ8AcVHBN5z/66kwAAAABJRU5ErkJggg==',
                'oldDataUrl': 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAAAXNSR0IArs4c6QAAAAZiS0dEAP8A/wD/oL2nkwAAAAlwSFlzAAALEwAACxMBAJqcGAAAAAd0SU1FB94MCBcTLdn9ugMAAAAdaVRYdENvbW1lbnQAAAAAAENyZWF0ZWQgd2l0aCBHSU1QZC5lBwAAA05JREFUeNrtm71u01AUx3/JTSktRZiP0oiFzkxdUIdOWZmKCkjd3CfoGzDwIpmrIp4AKS+AlGzMXRC0aomElH7XDD0R5pLEjhOfY1r/patWre17/j/7fvjcayhVqlSpUqVurSqKdVWljKozAq6k3CgADpgF5oGZIfVGwDnQA06By5sCwAFzwCMpc/I3X5fAMXAk5VgDQk3J/GNgSQDMSlPwdSV3/o7cmEMNCDUl83X5OSfmHwLPYsd+A35KPPF+IncINWXzDngKvAcWY8cfAB+A/dh5aEBwBTAPcA94CXwRs046SydN41wAREUHkMW8KQRXEPNmEJyl+eXlZbrdrikEZ2U+DENarRZ7e3t0Oh0zCM7KfLPZBGB9fd0UgrM035clBGdt3hqCK4J5SwiuKOatILgimbeA4IpmXhuCK6J5TQiuqOa1ILiEHN7dmPkn2uYngFAV82dJEFwCnPuSyVmUfJ66+YwQarEU2+moJGsaAIvAA7nokoX5MSH0JKN0BvwSCJmyzDOStloDXgNbkrGJ/BKGYaSpMAyjQXFIfFsS75rEP0NCO0+bOZ63uvO+ms0mYRgO+le/qabOeFcnDWZ7exsLTaveNABGDiONRmNQm8xVnU6HRqMxUdxpAcSXqvaBj/4B3W5XFULf/IBMEhLffizmaBIA/aWq+FCyawkhhfld/iywHEv8UVYAVzKk+EtVJhBSmveX13pJQ2ASgFO50A/+XqBQhTCm+UOJ9yhpEpR2qBj1PvAWeOOfEAQBrVaLlZUVC/PfGWM1Ke3qsAmEvM2PA0Adgob5cQGoQdAynwVA7hA0zWcFkBsEbfOTygELwHNgFXgFbADvJNB/3taCIIja7fbAN7x2ux0FQTDsLW9Xrrsh9axKvQvks8SvC+F/NT8VCNbmp3GRSNrfucy64snJr9LPvIifcHJyws7ODvV6nc3NTdM2X5nykzBWxzhCah3eNNvQ2E+CtXly6EQmhaA+1OXRi2aFYDLO5zWMJEG4kN8PpHwGPllMcvLeKzxqt2hlQOpNfYaX90TCfxKqXC+wVLx844WXzFCb3ua9WRrvsY64XrHxd4yb7BTXaAL+0zbsmwGTbwW0AfRzkMO+GlH/WqRUqVKlSt12/QabclQ7V4MgywAAAABJRU5ErkJggg==',
            },
            {
                'name': 'square',
                'dataUrl': 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAA4AAAAOBAMAAADtZjDiAAAABGdBTUEAALGPC/xhBQAAACBjSFJNAAB6JgAAgIQAAPoAAACA6AAAdTAAAOpgAAA6mAAAF3CculE8AAAAGFBMVEUAAAD////////////////b29s6OjoAAACbUO9ZAAAABHRSTlMAica662NszwAAAAFiS0dEAIgFHUgAAAAJcEhZcwAACxIAAAsSAdLdfvwAAAA0SURBVAjXY2AQcnFxUWRgYHYBAQMGkbDy8vJURwaVdCBd5sTgUg4CLkTTMH0wc2DmQu0BABQVIGmcCGKgAAAAAElFTkSuQmCC',
                'oldDataUrl': 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAAAXNSR0IArs4c6QAAAAZiS0dEAP8A/wD/oL2nkwAAAAlwSFlzAAALEwAACxMBAJqcGAAAAAd0SU1FB94MCBcREcSkpAYAAAAdaVRYdENvbW1lbnQAAAAAAENyZWF0ZWQgd2l0aCBHSU1QZC5lBwAAAaxJREFUeNrtms1OwkAURs/QCkIkGn+CS97AtQ/nipdz7QvoUhMgGkUEA3XR26QxuOiUzkj6nWRCAgHmnvm5zcwFIYQQQgghWomr+f2ONRep/xmwtRZcQAL0gAFwFEFCBnwDn8AK2IQUkAB94Nxa394LyQZYAnNrSx8JaY3gL4CRCejZUgjJ1ka+awM585GQeqz5ngU9Ai5NRifSEkhL4jNgCnxV2RN8BAx2TP0T4A4YBwr+CZgAH6WluLb9YF1FQOIx/YfAFXBqAoeBgwc4A26Ae9sInQX+bstgW2VEfTbOcvoLHXzB2P7b1UnHvhuX+9WRWIzrZrQOLSdt4kezLGvmsdXtP9G0fgZIgARIgARIgARIgARIgARIgARIgARIgARIgAS0jEYORZs4vNQM+GcCyufei4j9X/zRp0YFlKsyMvJLykWk4Cc7+lOJxENYl/yGuE9eGfIGPAC39lnI4B8t8CXwaq3S9bjzEHBMXhdwTfz6gCL4KfBMgPqAoipjXgo4doXIHHix1xUVC6Z80mBRmzOzUVhzwDVCqhLbQxo96DpBIYQQQgghRDv5AYMiaMSWRmsPAAAAAElFTkSuQmCC',
            },
            {
                'name': 'plus',
                'dataUrl': 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAA4AAAAOBAMAAADtZjDiAAAABGdBTUEAALGPC/xhBQAAACBjSFJNAAB6JgAAgIQAAPoAAACA6AAAdTAAAOpgAAA6mAAAF3CculE8AAAAGFBMVEUAAAD////////////////v7+8AAADg4OAzVv9cAAAABHRSTlMALMMRCKN9YQAAAAFiS0dEAIgFHUgAAAAJcEhZcwAACxIAAAsSAdLdfvwAAAA/SURBVAjXY2BgEHJRZAABFRcnEMXsmhZiAKSNgbQxSBZIA1WohIanlYY6MbikgYALnIaJw9TB9MHNgZkLsQcAd1oUdci9jDwAAAAASUVORK5CYII=',
                'oldDataUrl': 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAYAAACqaXHeAAAAAXNSR0IArs4c6QAAAAZiS0dEAP8A/wD/oL2nkwAAAAlwSFlzAAALEwAACxMBAJqcGAAAAAd0SU1FB94MCBcSNz2EcjgAAAAdaVRYdENvbW1lbnQAAAAAAENyZWF0ZWQgd2l0aCBHSU1QZC5lBwAABHZJREFUeNrtm79PY0cQxz9+ts9AfpxzP3ToKvsqlAaKnJQi0uHOqcw1V1OkIR1KyqRI0kZCSh3J6KoUUbj/AJd0cEVOpglGSgHxHeIuOWww9kvBPGnBP96u366NxRtpZWn9PLvz3dnZ+c5bQyyxxBJLLDdXEiMYw5NmOpYPdKRNLABJIAPMAGmD8XygBZwAp0B7EgFIAtPAHWnT0qcjbaABHElruAIh5dj4u8ADASAjW0FHOrLyt2SR3rgCIeXY+Fn5nDaMA77MTf2NExBsA+DJSt+Rlb+ruH4e+ExTzyvgTwVIX9proGkzMLoAYKbHvs8D3wMfaOppAT8DO0ocOZOgeGYTgKQD9/8IuA/cFoAfGRof6Pkc+As4lG1wBvwr26Bjc8VcnCzq2f+lofGBpIFSD33WXXZSjlcnR7arY3CgLC0tMT8/f6lvf3+f9fX1kc9lLACUSiWWl5cv9VUqlbEA4Fl20YTj7NL6GKkhAOsXjBISuFIOY0BKxgiSpcgEygSAMGKTkGc+lucSlo3PiG5fskHfBoFKGRivQ2w8yd9nLOcYSdH5APikzwoPRaBSBsbrEpvAE2zt1UBHRubr2yRQKY09r+b29zSJTWIMwU8lUOhyBx0AeuX2t4BvgXnHp9SnwG8az9WAn4D/TLmDp4F8WpRmlD0+CuNNJCd840PFa6d1qlCepvupx99XUY3P5XJdfbOzs0xNTUUF4RtT7uAZBiKE6Q0t5XKZxcXFrv65uTk2NjaignDfNA55o/TTcrnclQKrUiwWbYAwei6wubnZc1WHkWKxSKPRCI96tRr5fP7a0uGJkRiAGIAbLlaC4OrqKtlsNvS5tbU1FhYWQoPbysoKzWZz4HNh39uSNPAQ+AJ4Cjzjol7vD9Oy2ay/vb3t95O9vT0/l8v5w+oH/pE5PpU5P1TqB5G2gMrAWsOieXx8TKFQoFqtdn13cHBAoVCgVqtFWbBWnzlHAkCtsvjAr0A9CghbW1td/dVqNarx74Ffesw3EgBBlaUhXLsjbvZjFBAcyHthg3tKXaAhc/ejBMGOUMojYYGesKxD4DvZY2HcoRSBPNWA5xpuXZeF6VypCp0QUh/UAeBUlAVGBRWheh8vuFoRehJhZU+4eFEaBLm2RkXoSBboSPHaSMdgUGt7I4OfoVcTzFhy78D405ACh7OaoKr8tUxiUFX4tpTPUth78dKWcQ+Bt4yhKhxMoikrMOi9QALI6h5DBl5wCrwTT+wX3Jy+FyBEeQDKuWXjVePOxfiWrTE8yxP0HRnvbIyxvBzd3d2lUqlc6tvZ2RlLAmG7fp/moi73SD7TwNcRjsJXwA/i8nUubozUo6Tjo6LD/oTodALA1Vz8ZQRdL01ze1NJOgA0SISCFxN/S/7w2FDX78AfSnp7LO1aX5Prxx2CiPfEYOVf9MjuTrB8edrFRQZbN0U7Sgp+gKOboklHMaAtkbojhge3RtT9PKidK8YfMmF3hU0JFLaIzXXZAlc97Mb+X0A9Ga7tP0ZiiSWWWGKJ5QbL/3MWtbxJYUX0AAAAAElFTkSuQmCC',
            },
            {
                'name': 'circle',
                'dataUrl': 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAA4AAAAOBAMAAADtZjDiAAAABGdBTUEAALGPC/xhBQAAACBjSFJNAAB6JgAAgIQAAPoAAACA6AAAdTAAAOpgAAA6mAAAF3CculE8AAAAJ1BMVEUAAAD///////////////////////////+lpaU2NjYFBQVqamoAAABhEXpWAAAAB3RSTlMAHo3J+1nwQd7ULAAAAAFiS0dEAIgFHUgAAAAJcEhZcwAACxIAAAsSAdLdfvwAAABLSURBVAjXY2BgVHYxEmBgYAgrLy9PZWBgLe9cNaM8gEGs+syZM9sTGdR7gPSJIgbzOUD6ZDGD+xogfaoETsPEYepg+mDmwMyF2gMAIKoufQwFFCkAAAAASUVORK5CYII=',
                'oldDataUrl': 'data:image/png;base64,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',
            },
            {
                'name': 'x',
                'dataUrl': 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAA4AAAAOBAMAAADtZjDiAAAABGdBTUEAALGPC/xhBQAAACBjSFJNAAB6JgAAgIQAAPoAAACA6AAAdTAAAOpgAAA6mAAAF3CculE8AAAAJ1BMVEUAAAD///////////////////////+9vb02Njbq6uoAAAAsLCwoKChymQDvAAAABnRSTlMABo3J2KtoTgRvAAAAAWJLR0QAiAUdSAAAAAlwSFlzAAALEgAACxIB0t1+/AAAAFBJREFUCNdjYFRWYGAyEmAQSUtlCEtzZFCrmBaa2Z7EYNa1M232imQGtdmrZ+7amcTAmrnrzOppAQwMYXNWnUxlgNMwcZg6uD6oOTBzofYAAD/OJduXWwPsAAAAAElFTkSuQmCC',
                'oldDataUrl': 'data:image/png;base64,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',
            }
        ],
        colors: [
            '#FF2B2D',
            '#FF6003',
            '#FFA801',
            '#FFDD20',
            '#F2FF01',
            '#A8FF00',
            '#00C305',
            '#14F5CD',
            '#33E0FE',
            '#28BAEB',
            '#247BFE',
            '#6B96FE',
            '#9126FF',
            '#CF0EFF',
            '#FF7EDA',
            '#FE1370',
            '#E5E5E5',
            '#BFBFBF',
            '#808080',
            '#000000',
            'rgba(0,0,0,0.0)'
        ],
        palettes: [
            {
                Steps: [
                  { Value: 0.0, Color: "#FF1C8C" },
                  { Value: 0.5, Color: "#FFFDE4" },
                  { Value: 1.0, Color: "#1A82F6" }
                ]
            },
            {
                Steps: [
                  { Value: 0.0, Color: "#000" },
                  { Value: 0.5, Color: "#F00" },
                  { Value: 1.0, Color: "#FF0" }
                ]
            },
            {
                Steps: [
                  { Value: 0.0, Color: "#330083" },
                  { Value: 0.2, Color: "#9B0093" },
                  { Value: 0.5, Color: "#EC0929" },
                  { Value: 0.75, Color: "#F35633" },
                  { Value: 1.0, Color: "#FFFD4D" }
                ]
            },
            {
                Steps: [
                  { Value: 0.0, Color: "#004072" },
                  { Value: 0.25, Color: "#0CA6FF" },
                  { Value: 0.5, Color: "#A6E1FF" },
                  { Value: 0.75, Color: "#FFEC6D" },
                  { Value: 1.0, Color: "#FFF000" }
                ]
            },
            {
                Steps: [
                  { Value: 0.0, Color: "#3C0060" },
                  { Value: 0.25, Color: "#3C0060" },
                  { Value: 0.25, Color: "#9D01DC" },
                  { Value: 0.75, Color: "#9D01DC" },
                  { Value: 0.75, Color: "#00FEB6" },
                  { Value: 1.0, Color: "#00FEB6" }
                ]
            },
            {
                Steps: [
                { Value: 0.0, Color: "#000" },
                { Value: 1.0, Color: "#FFF" }
                ]
            }
        ],
        toRgba: function (color) {
            //if this already looks like a color object...
            if (angular.isObject(color) && "r" in color && "g" in color && "b" in color && "a") {
                return angular.copy(color);
            }

            //try to parse #808080 type of colors
            var hex = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(color);
            if (hex) {
                return {
                    r: parseInt(hex[1], 16),
                    g: parseInt(hex[2], 16),
                    b: parseInt(hex[3], 16),
                    a: 1.0
                }
            }

            //try to parse rgb(255,10,10,0.3) type of colors
            var rgb = /^rgb\s*\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)$/i.exec(color);
            if (rgb) {
                return {
                    r: parseInt(rgb[1]),
                    g: parseInt(rgb[2]),
                    b: parseInt(rgb[3]),
                    a: 1.0
                }
            }

            //try to parse rgba(255,10,10,30%) type of colors
            var rgbaPercent = /^rgba\s*\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*([\d\.]+)\%\s*\)$/i.exec(color);
            if (rgbaPercent) {
                return {
                    r: parseInt(rgbaPercent[1]),
                    g: parseInt(rgbaPercent[2]),
                    b: parseInt(rgbaPercent[3]),
                    a: parseFloat(rgbaPercent[4]) / 100
                }
            }

            //try to parse rgba(255,10,10,0.3) type of colors
            var rgba = /^rgba\s*\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*([\d\.]+)\s*\)$/i.exec(color);            
            if (rgba) {
                return {
                    r: parseInt(rgba[1]),
                    g: parseInt(rgba[2]),
                    b: parseInt(rgba[3]),
                    a: parseFloat(rgba[4])
                }
            }
            return undefined;
        },
        toCss: function (c) {
            if (c.a === 1.0) {
                return "#" + ((1 << 24) + (c.r << 16) + (c.g << 8) + c.b).toString(16).slice(1).toUpperCase();
            } else {
                return "rgba(" + c.r + "," + c.g + "," + c.b + "," + c.a + ")";
            }
        },
        setAlpha: function (c,a) {
            var color = this.toRgba(c);
            color.a = a;
            return this.toCss(color);
        }
    };
});
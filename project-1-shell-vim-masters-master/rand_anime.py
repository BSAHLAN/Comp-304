import requests
from bs4 import BeautifulSoup
import random

response = requests.get('https://myanimelist.net/topanime.php')

page = response.text
soup = BeautifulSoup(page, "html.parser")
rankings = soup.find_all("tr",class_="ranking-list")

animeList = []
for i in rankings:
    anime = i.text
    animeList.append(anime.replace("\n", "/"))
 
rand_anime = random.choice(animeList)

rand_anime_split = rand_anime.split("/")
rand_anime_split = [i.strip() for i in rand_anime_split if i != ""]

display = "Name: " + rand_anime_split[1] +  "\nRank: " + rand_anime_split[0] + "\nPlatform and number of episodes: " + rand_anime_split[2] + "\nSeason: " + rand_anime_split[3] + "\nRate: " + rand_anime_split[6]

print(display)


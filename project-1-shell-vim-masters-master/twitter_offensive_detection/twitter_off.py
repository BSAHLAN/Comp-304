import sys
import os 
import ktrain
import warnings
import requests
from bs4 import BeautifulSoup
## need to download krain, tensorflow using conda and activate conda environment using source Path_OF_ENV/bin/activate to use command
# STEP1: pip install conda , careate a conda environmet using command : conda create --name myenv python=3.11. activate using source Path_OF_ENV/bin/activate
# STEP2: pip install ktain, pip install bs4 , pip install tensorflow
# STEP3: HAVE FUN !! (the reason I use conda us ktrain had some issues )

# class so the model does not load evey time it is called
class Singleton(type):
    _instances = {}

    def __call__(cls, *args, **kwargs):
        if cls not in cls._instances or kwargs.get('model_fn'):
            cls._instances[cls] = super(Singleton, cls).__call__(*args, **kwargs)
        return cls._instances[cls]
# model class, we use it to predict the if the text is 
class berturk_off(metaclass=Singleton):

    def __init__(self):

        home_dir = os.path.expanduser("~")# this is asuming your using it under the come directory, please
        model_dir = os.path.join(home_dir,"project-1-shell-vim-masters/twitter_offensive_detection/turkish_off_uncased")# if this does not work please copy the abolute path to the model directory 
        clf = ktrain.load_predictor(model_dir)
        self.clf = clf
    def prediction(self,text):
        sentences = [text]
        label = self.clf.predict(text)
        return label

def predict_OFF(text):

    clf = berturk_off()
    label = clf.prediction(text)
    return label

# get tweets if user
def get_tweets(userName, count=5): 
    url = f"https://nitter.net/{userName}"
    response = requests.get(url,headers={"User-Agent":"Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Safari/537.36",
    "Accept":"text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7",
    "Accept-Encoding":"gzip, deflate, br",
    "Accept-Language":"en-US,en;q=0.9",
    "Sec-Ch-Ua":'"Google Chrome";v="119", "Chromium";v="119", "Not?A_Brand";v="24"'})

    
    tweets = []

    if(response.status_code == 200):
        soup = BeautifulSoup(response.text,'html.parser')
        div = soup.find_all('div',class_="tweet-body")
        for div_elm in div[:count]:
            text = div_elm.find_all('div',"tweet-content media-body")
            for tweet_text in text:
                tweets.append(tweet_text.text )              
    
    
    return tweets





def main():
   if len(sys.argv) > 1:
        
        if(sys.argv[1] == "r"):#where we call with paramter "r" and "userName" get only the first 5 tweets
            user = sys.argv[2]
            tweets = get_tweets(user)
            print("--------------------------------")
            for tw in tweets:
                print(tw)
                print("--------------------------------")
        # if no flag has been given then prints out if last 5 tweets are offensive
        argument = sys.argv[1]
        tweets = get_tweets(argument)
       
        for tw in tweets:
            
            output = predict_OFF(tw)
            print(tw + " --->" + output)
            print("--------------------------------")

       

    

# Check if the script is being run directly
if __name__ == "__main__":

    warnings.filterwarnings("ignore")    
    main()





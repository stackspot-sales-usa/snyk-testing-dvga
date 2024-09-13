from modules import *
import requests
import json
import os
import sys
import concurrent.futures
import time

client_id = get_env_variable("STK_CLIENT_ID")
client_secret = get_env_variable("STK_CLIENT_SECRET")
realm = get_env_variable("STK_CLIENT_REALM")
commit_sha = get_env_variable("GITHUB_SHA")
gh_access_token = get_env_variable("GH_TOKEN")
JIRA_API_TOKEN = get_env_variable("JIRA_API_TOKEN")

stk_access_token = get_stk_bearer_token(client_id, client_secret, realm)
qc_slug = "poc-foxconn"
repo_owner = "stackspot-sales-usa"
repo_name = "mvp-sec-issue-ticket"

repo_path = os.getenv("GITHUB_WORKSPACE", ".")
code_dict = read_all_files_in_repo(repo_path)

with concurrent.futures.ThreadPoolExecutor() as executor:
    futures = [
        executor.submit(
            process_file, file_name, file_code, stk_access_token, qc_slug, repo_owner, repo_name, gh_access_token, JIRA_API_TOKEN
        )
        for file_name, file_code in code_dict.items()
    ]
    for future in concurrent.futures.as_completed(futures):
        try:
            future.result()
        except Exception as e:
            print(f"Error in future: {e} (this is a parallel processing error)")

print("EOF")
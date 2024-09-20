from modules import *
import requests
import json
import os
import sys
import concurrent.futures
from requests.auth import HTTPBasicAuth
import time


def get_env_variable(var_name):
    value = os.getenv(var_name)
    if not value:
        print(f"Missing required environment variable: {var_name}")
        sys.exit(1)
    return value

def get_stk_bearer_token(client_id, client_secret, realm):
    url = f"https://idm.stackspot.com/{realm}/oidc/oauth/token"
    headers = {
        "Content-Type": "application/x-www-form-urlencoded"
    }
    data = {
        "client_id": client_id,
        "grant_type": "client_credentials",
        "client_secret": client_secret
    }
    
    response = requests.post(url, headers=headers, data=data)
    response_data = response.json()
    #print(response_data)
    stk_access_token = response_data.get("access_token")
    
    if stk_access_token:
        return stk_access_token
    else:
        raise Exception("Failed to retrieve access stk_access_token")

def create_rqc_execution(qc_slug, stk_access_token, input_data, file_name):
    url = f"https://genai-code-buddy-api.stackspot.com/v1/quick-commands/create-execution/{qc_slug}"
    headers = {
        'Content-Type': 'application/json',
        'Authorization': f'Bearer {stk_access_token}'
    }
    data = {
        'input_data': input_data
    }
    response = requests.post(url, headers=headers, json=data)
    if response.status_code == 200:
        decoded_content = response.content.decode('utf-8')  # Decode bytes to string
        extracted_value = decoded_content.strip('"')  # Strip the surrounding quotes
        response_data = extracted_value
        print(f'{os.path.basename(file_name)} ExecutionID:', response_data)
        return response_data
    else:
        print(f'{file_name} stackspot create rqc api response:{response.status_code}')
        return None

def get_execution_status(execution_id, stk_access_token,file_name):
    url = f"https://genai-code-buddy-api.stackspot.com/v1/quick-commands/callback/{execution_id}"
    headers = {'Authorization': f'Bearer {stk_access_token}'}
    i = 0

    response = requests.get(url, headers=headers)
    while True:
        response = requests.get(url, headers=headers)
        response_data = response.json()
        status = response_data['progress']['status']
        if status in ['COMPLETED', 'FAILURE']:
            print(f"{os.path.basename(file_name)}: Execution complete!")
            return response_data
        else:
            print(f"{os.path.basename(file_name)}: Status:", f'{status} ({i} seconds elapsed)')
            print(f"{os.path.basename(file_name)}: Execution in progress, waiting...")
            i += 5
            time.sleep(5)  # Wait for 5 seconds before polling again

def execute_qc_and_get_response(stk_access_token, qc_slug,input_data, file_name):
    execution_id = create_rqc_execution(qc_slug, stk_access_token, input_data, file_name)
    if execution_id:
        execution_status = get_execution_status(execution_id, stk_access_token,file_name)
        if execution_status['progress']['status'] == "FAILURE":
            print(f"Execution failed for file: {file_name} (Execution ID: {execution_id})")
        return execution_status
    else:
        return None
    
def process_file(file_name, file_code, stk_access_token, qc_slug, repo_owner, repo_name, gh_access_token, JIRA_API_TOKEN):
    print(f"Started processing file: {os.path.basename(file_name)}")
    if not file_code:  # Check if the file code is empty
        print(f"Skipping empty file: {file_name}")
        return

    try:
        response = execute_qc_and_get_response(stk_access_token, qc_slug, file_code, file_name)
    except Exception as e:
        print(f"Error processing file {file_name}: {e}")
        print(f'This was the response from Stackspot AI: {response}')
        return

    print(f"{os.path.basename(file_name)} has been PROCESSED")
    # in my rqc step 3 represents if it secure, true, or unsecure, false.
    issue_dict=process_api_response_to_issue_dict(response, os.path.basename(file_name))
    for title,body in issue_dict.items():
        '''title=json.dumps(title)
        body=json.dumps(body)'''

        create_jira_issue(title, body, JIRA_API_TOKEN, file_name)


def sanitize_code(code):
    # Remove comments and strip extra whitespace
    sanitized_lines = []
    for line in code.split('\n'):
        # Remove comments
        line = line.split('#')[0]
        # Strip extra whitespace
        line = line.strip()
        if line:  # Only add non-empty lines
            sanitized_lines.append(line)
    return '\n'.join(sanitized_lines)


def create_github_issue(repo_owner, repo_name, title, body, gh_access_token):
    url = f"https://api.github.com/repos/{repo_owner}/{repo_name}/issues"
    headers = {
        "Authorization": f"token {gh_access_token}",
        "Accept": "application/vnd.github.v3+json"
    }
    data = {
        "title": title,
        "body": body
    }
    
    try:
        response = requests.post(url, headers=headers, json=data)
        response.raise_for_status()  # Raise an HTTPError for bad responses (4xx and 5xx)
    except requests.exceptions.HTTPError as http_err:
        print(f"HTTP error occurred: {http_err}")
        print(response.json())
        return None
    except Exception as err:
        print(f"An error occurred: {err}")
        return None
    else:
        print("Github issue created successfully.")
        return response.json()

def read_all_files_in_repo(repo_path):
    # Get the GitHub workspace directory
    github_workspace = os.getenv('GITHUB_WORKSPACE', repo_path)
    # Construct the full path to the define-scannable-files.txt file
    scannable_files_path = os.path.join(github_workspace, 'scripts/define-scannable-files.txt')
    # Read the scannable file extensions from the define-scannable-files.txt file
    with open(scannable_files_path, 'r') as f:
        scannable_extensions = [line.strip() for line in f.readlines() if line.strip()]
    code_dict = {}
    for root, dirs, files in os.walk(repo_path):
        
        for file in files:
            file_extension = os.path.splitext(file)[1]
            if file_extension not in scannable_extensions:
                continue
            
            print(f'THIS FILE WILL BE ANALYZED:{file}')

            file_path = os.path.join(root, file)
            
            with open(file_path, 'r', encoding='utf-8') as f:
                code_dict[file_path] = sanitize_code( f.read())

    return code_dict

def get_commit_files(repo_owner, repo_name, commit_sha, gh_access_token):
    commit_url = f"https://api.github.com/repos/{repo_owner}/{repo_name}/commits/{commit_sha}"
    headers = {'Authorization': f'token {gh_access_token}'}
    response = requests.get(commit_url, headers=headers)
    response.raise_for_status()
    commit_data = response.json()
    files = commit_data['files']
    
    result = {}
    for file in files:
        filename = file['filename']
        file_url = f"https://api.github.com/repos/{repo_owner}/{repo_name}/contents/{filename}?ref={commit_sha}"
        file_response = requests.get(file_url, headers=headers)
        file_response.raise_for_status()
        file_content_data = file_response.json()
        filecode = requests.get(file_content_data['download_url']).text
        print(f"Adding file: {filename}")
        #print(f"file's code: {filecode}")
        result[filename] = filecode
    
    return result

def create_jira_issue(issue_title, issue_description, JIRA_API_TOKEN,file_name):
    JIRA_INSTANCE_URL = 'https://stackspot-sales-us.atlassian.net'
    USERNAME = 'lucas.vicenzotto@stackspot.com'
    PROJECT_KEY = 'POC'
    # Get the API token from the environment variable
    if not JIRA_API_TOKEN:
        raise ValueError("JIRA_API_TOKEN environment variable not set")
    url = f"{JIRA_INSTANCE_URL}/rest/api/2/issue"
    auth = HTTPBasicAuth(USERNAME, JIRA_API_TOKEN)
    headers = {
        "Content-Type": "application/json"
    }
    payload = {
        "fields": {
            "project": {
                "key": PROJECT_KEY
            },
            "summary": issue_title,
            "description": issue_description,
            "issuetype": {
                "name": "Task"
            }
        }
    }
    response = requests.post(url, headers=headers, auth=auth, data=json.dumps(payload))
    if response.status_code == 201:
        print("Jira issue created successfully.")
        jira_issue = response.json()
        issue_key = jira_issue.get('key')
        
        # Construct the Jira issue URL
        jira_issue_url = f"{JIRA_INSTANCE_URL}/browse/{issue_key}"
        print(f"{os.path.basename(file_name)} Jira Issue URL: {jira_issue_url}")
        
        return jira_issue
    else:
        # Handle failure to create the issue
        print(f"Failed to create Jira issue. Status code: {response.status_code}")
        print(response.text)
        return None

def get_pull_request_files(repo_owner, repo_name, pull_number, github_token):
    # GitHub API URL to get the list of files in a pull request
    url = f"https://api.github.com/repos/{repo_owner}/{repo_name}/pulls/{pull_number}/files"
    headers = {
        "Authorization": f"token {github_token}",
        "Accept": "application/vnd.github.v3+json"
    }
    response = requests.get(url, headers=headers)
    
    if response.status_code != 200:
        raise Exception(f"Failed to fetch pull request files: {response.status_code} {response.text}")
    
    files = response.json()
    print(f'These are the files from the PR: {files}')
    code_dict = {}
    
    for file in files:
        file_name = file['filename']
        # Construct the raw URL manually
        raw_url = f"https://raw.githubusercontent.com/{repo_owner}/{repo_name}/{file['sha']}/{file_name}"
        print(f"Fetching content from: {raw_url}")
        
        file_response = requests.get(raw_url, headers=headers)
        if file_response.status_code != 200:
            raise Exception(f"Failed to fetch file content for {file_name}: {file_response.status_code} {file_response.text}")
        
        file_code = file_response.text
        code_dict[file_name] = file_code
    
    return code_dict

def get_last_pull_request_number(repo_owner, repo_name, github_token):
    url = f"https://api.github.com/repos/{repo_owner}/{repo_name}/pulls?state=all&sort=created&direction=desc"
    headers = {
        "Accept": "application/vnd.github.v3+json",
        "Authorization": f"token {github_token}"
    }
    response = requests.get(url, headers=headers)
    response.raise_for_status()
    pull_requests = response.json()
    if pull_requests:
        return pull_requests[0]['number']  # Return the number of the most recent pull request
    else:
        return None

def process_api_response_to_issue_dict(response, file_name):
    # Execute the Quick Command and get the response
    unfiltered_result = response.get('result')

    try:
        # Remove leading and trailing backticks and "json" tag if present
        if unfiltered_result.startswith("```json"):
            unfiltered_result = unfiltered_result[7:-4].strip()
        result_list = json.loads(unfiltered_result)
    except json.JSONDecodeError as e:
        print(f"Invalid JSON format: {e}")
    i=0
    issue_dict={}
    for body in result_list:
        i+=1
        title=f'Issue #{i} in file:{os.path.basename(file_name)}'
        issue_dict[title]=body
    return issue_dict
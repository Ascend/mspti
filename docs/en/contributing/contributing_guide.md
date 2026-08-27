# Contributing to the MindStudio Profiler Tools Interface

Thank you for considering contributing to the MindStudio Profiler Tools Interface (msPTI)! We welcome all forms of contribution, including bug fixes, feature enhancements, documentation improvements, and even just feedback. Whether you are an experienced developer or contributing to an open-source project for the first time, your help is invaluable.

You can support this project in several ways:

- Report issues through [Issues](https://gitcode.com/Ascend/mspti/issues).
- Suggest or implement new features.
- Improve or extend the documentation.
- Review pull requests and assist other contributors.
- Spread the word: share msPTI in blog posts, on social media, or give the repository a star.

## Finding Issues to Contribute To

Want to get started? Look for the following types of issues:

- Good first issues
- Call for contribution

You can also learn about the development plans and roadmap of the project by browsing the [Issues list](https://gitcode.com/Ascend/mspti/issues).

## Contribution Workflow

### Environment Requirements

- For the hardware environment, see the [Ascend Product Form Description](https://www.hiascend.com/document/detail/en/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html).
- Install the CANN open-source version in advance.
- Python 3.8 or later
- CMake 3.14 or later

### Development and Testing

1. Fork the source code into your personal repository for development

   ```bash
   git clone https://gitcode.com/<your-username>/mspti.git
   cd mspti
   ```

2. Develop the code in your personal repository

   Follow the [code standards](#code-standards) during development.

3. Test the code

   See [code testing](#code-testing).

4. Compile and test

   Compile the developed code into a whl package for testing. For detailed steps, see the [msPTI Tool Installation Guide](../install_guide/mspti_install_guide.md).

5. Develop documentation

   If your changes add, modify, or remove features, provide the related documentation. For detailed documentation requirements, see [documentation development](#documentation-development).

6. Submit a pull request

   See [pull request submission process](#pull-request-submission-process).

### Code Standards

#### Python Code Standards

- Follow the PEP 8 coding standards
- Use four spaces for indentation
- Use PascalCase for class names (for example, `DataManager`)
- Use snake_case for function and variable names (for example, `parse_data`)
- Add the necessary type annotations and docstrings

#### C++ Code Standards

- Follow the existing coding style of the project
- Use four spaces for indentation
- Use PascalCase for class names
- Use camelCase for function names
- Add the necessary comments to explain complex logic

### Code Testing

#### Running Tests

Before submitting your code, ensure that all tests pass:

```bash
bash scripts/execute_test_case.sh
```

#### Adding Tests

- Add corresponding unit tests for new features
- Ensure that tests cover the main logic branches
- Keep test cases readable and maintainable
- Place test data in the corresponding location in the `test/` directory

### Documentation Development

#### Documentation Paths

If your changes affect how users use the product, update the related documentation:

- User guide: `docs/en/`
- API documentation: docstrings in code comments
- Sample code: `samples/`

#### Documentation Standards

- Use clear and concise Chinese
- Provide complete sample code
- Include the necessary screenshots or diagrams
- Ensure that links remain valid

### Pull Request Submission Process

#### Pre-Submission Checklist

Before submitting a pull request, ensure that:

- [ ] Code follows the coding standards of the project
- [ ] The necessary test cases are added
- [ ] All tests pass
- [ ] The related documentation is updated
- [ ] The commit message is clear and concise
- [ ] The code has been self-reviewed

#### Submission Process

1. **Creating a branch**

   ```bash
   git checkout -b feature/<your-feature-name>
   ```

2. **Committing your changes**

   ```bash
   git add .
   git commit -m "feat: <your feature description>"
   ```

3. **Pushing to the remote repository**

   ```bash
   git push origin feature/<your-feature-name>
   ```

4. **Creating a pull request**

   Create a pull request on GitCode and fill in the following:

   1. A clear title

      Follow the [commit message standards](#commit-message-standards).

   2. A detailed description

      Include the changes made, the reasons for them, the test results, and so on.

   3. Link the related issue

5. **Code review**

   1. After submitting the pull request, notify the relevant "owners" (Reviewers and Committers) to review the content.
   2. Modify the code according to the review feedback and resubmit the updates. This process may involve multiple rounds of iteration. Therefore, respond promptly and keep communicating.

   The pull request workflow prompts you to specify the relevant "owners". You can designate the "owners" in the pull request workflow, or contact us through the [README_EN](../../../README_EN.md#-suggestions-and-communication).

6. **Merging the code**

   To complete the merge, the pull request must obtain the following four labels in sequence:

   1. ascend-cla/yes: CLA check. You must sign the CLA when contributing for the first time. After that, every submission automatically receives this label.
   2. ci-pipeline-passed: CI pipeline. Trigger it by commenting `compile` in the pull request workflow. If the CI pipeline check fails, modify the code according to the prompts and resubmit it.
   3. lgtm: Provided by Reviewers. After the Reviewers approve the pull request, they comment `/lgtm` in the pull request workflow to trigger the lgtm label.
   4. approved: Provided by Committers. After the Committers approve the pull request, they comment `/approved` in the pull request workflow to trigger the approved label.

   Once your pull request obtains all four labels, your PR is merged into the main branch.

#### Pull Request Best Practices

- Keep PRs reasonably sized so that they are easy to review
- Use one PR for one problem or feature
- Respond to review feedback promptly
- Keep your branch in sync with the main branch and resolve conflicts promptly

#### Commit Message Standards

A commit message should clearly describe what changed and why:

```text
<type>: <subject>

<body>

<footer>
```

The type can be one of the following:

- `feat`: new feature
- `fix`: bug fix
- `docs`: documentation update
- `style`: code formatting adjustments (no impact on functionality)
- `refactor`: code refactoring
- `test`: test-related changes
- `chore`: changes to the build process or auxiliary tools

Example:

```text
feat: add memory usage analysis functionality

- Implement the memory data collection module
- Add the memory usage trend analysis algorithm
- Update the related documentation

Closes #123
```

## Community Guidelines

### Code of Conduct

We are committed to providing a friendly, safe, and inclusive environment for all participants. By participating in this project, you agree to the following:

- Respect different viewpoints and experiences
- Accept constructive criticism
- Focus on what is best for the community
- Show empathy toward other community members

### Communication Channels

- **Issues**: report bugs, suggest features, and discuss technical problems.
- **Pull Requests**: review code and discuss specific implementations.
- **WeChat group**: daily communication and quick Q&A (see the [README_EN](../../../README_EN.md#-suggestions-and-communication))

## License

By contributing code to this project, you agree that your contributions are licensed under the project license. For details, see the [LICENSE](../../../LICENSE) file.

Documentation in the msPTI docs directory is licensed under CC-BY 4.0. For details, see the [docs/LICENSE](../../LICENSE) file.

## Acknowledgments

Thank you for contributing to msPTI. Your efforts make this project stronger and more user-friendly. We look forward to your participation!

---

If you have any questions or need help, feel free to ask in [Issues](https://gitcode.com/Ascend/mspti/issues) or contact us through other community channels.
